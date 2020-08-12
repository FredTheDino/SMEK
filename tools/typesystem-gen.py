#!/usr/bin/env python3

import glob
from lark import lark, Visitor
from itertools import chain
import re
from string import Template
import copy


class Struct():
    """A struct or a class in C/C++.

    Can be used in a tree since it knows about its base.

    Members:
      - name        : str
      - parent      : Struct
      - source      : str
      - fields      : [{"TYPE": <type>, ...}, ...]
    """
    def __init__(self, name, parent=None):
        self.name = name
        self.parent = parent
        self.source = ""
        self.fields = []

    def parents_contain(self, name):
        """Return whether this struct/class inherits from `name`."""
        if self.parent is None:
            return False
        elif name == self.parent.name:
            return True
        else:
            return self.parent.parents_contain(name)

    def get_parents(self, name):
        """Returns a list with all the parents, from first to last"""
        if self.parent is None:
            return []
        elif name == self.parent.name:
            return [self.parent.name]
        else:
            return [self.parent.name] + self.parent.get_parents(name)

    def __str__(self):
        if self.parent is None:
            return self.name
        else:
            return f"{self.name} : {self.parent}"


def process_source(src):
    """Remove C-style comments from a string."""
    src += "\n"
    res = ""
    index = 0
    l = len(src)
    while index < l:
        prev_index = index
        if src[index:index+2] == "//":
            index = src.index("\n", index) + 1
        elif src[index:index+2] == "/*":
            index = src.index("*/", index) + 2
        else:
            cands = [src.find("//", index), src.find("/*", index), l]
            index = min([cand if cand != -1 else l for cand in cands])
            res += src[prev_index:index]
    return res


def find_struct_source(src, index):
    """Parse a string and return its top level Struct objects."""
    prev_index = index
    index = src.index("{", index) + 1
    struct = src[prev_index:index]
    brace_depth = 1
    while brace_depth != 0:
        assert src.find("}", index) != -1, "Unclosed struct"
        if src.find("{", index) == -1:
            for _ in range(brace_depth):
                prev_index = index
                index = src.find("}", index) + 1
                struct += src[prev_index:index]
                brace_depth -= 1
            break
        if src.find("}", index) < src.find("{", index):
            prev_index = index
            index = src.find("}", index) + 1
            struct += src[prev_index:index]
            brace_depth -= 1
        else:
            prev_index = index
            index = src.find("{", index) + 1
            struct += src[prev_index:index]
            brace_depth += 1
    return struct + ";"


def find_structs(paths):
    """Return a (name: Struct)-dictionary of all structs found in some files."""
    expr = re.compile(r"(class|struct)\s+(\w*)(?:\s*:\s*(?:(public|protected|private)\s+(\w*)|(\w*)))?\s*{",
                      re.ASCII | re.MULTILINE)
    # (class|struct)
    # \s+
    # (\w*)
    # (?:
    #   \s*
    #   :
    #   \s*
    #   (?:
    #       (public|protected|private)
    #       \s+
    #       (\w*)
    #     |
    #       (\w*)
    #   )
    # )?
    # \s*
    # {

    # each match contains ( entire match,
    #                       class/struct
    #                       name,
    #                       access modifier,
    #                       parent name if access modifier is present,
    #                       parent name if no access modifier is present
    #                     )
    # e.g.: [ (...), ('struct', 'A', 'public', 'Entity', ''),
    #         (...), ('struct', 'E', 'private', 'F', ''),
    #         (...), ('struct', 'B', '', '', 'Entity'),
    #         (...), ('struct', 'C', '', '', 'B')
    #       ]

    structs = {}
    for path in paths:
        with open(path, "r") as f:
            source = process_source("".join(f.readlines()))
            last_struct_end = 0
            for match in expr.finditer(source):
                if not match[2]:  # anonymous struct
                    continue
                if match.start() < last_struct_end:  # struct is inside already checked struct
                    continue
                if match[3]:  # access modifier present
                    name, parent = match[2], match[4]
                else:
                    name, parent = match[2], match[5]
                if name == "TestEnt": continue
                structs[name] = Struct(name, parent)
                structs[name].source = find_struct_source(source, match.start())
                last_struct_end = match.start() + len(structs[name].source)

    left = set(structs.keys())
    while left:
        struct = structs[left.pop()]
        if struct.parent in structs:
            struct.parent = structs[struct.parent]
        else:
            struct.parent = None
    return structs

class MetaDataExtractor(Visitor):
    def __init__(self):
        self.fields = []

    def field(self, field):
        if "static" not in field.children:
            d = {}
            for token in field.children:
                d[token.type] = token.value.strip()
            self.fields.append(d)

def to_enum(name):
    return name.upper()

if __name__ == "__main__":
    lexer = lark.Lark.open("tools/struct.lark", start="structs")
    base_entity = "BaseEntity"
    #TODO(gu) parse entire files instead of one at a time
    all_structs = find_structs(glob.glob("src/**/*.*", recursive=True))
    entity_structs = { name: struct for name, struct in sorted(all_structs.items()) if struct.parents_contain(base_entity) or name == base_entity }
    for name, struct in entity_structs.items():
        # Parse out the structs
        tree = lexer.parse(struct.source)
        struct_types = list(tree.find_data("struct_type"))
        if len(struct_types) == 0:
            print("No typename found, 1 required")
            continue
        if len(struct_types) > 1:
            #TODO(gu) parse structs in structs correctly
            print(f"{len(struct_types)} typenames found (structs in structs?), 1 required\nstructs are {struct_types}")
            continue

        # Parse out the field meta information
        meta_data = MetaDataExtractor()
        meta_data.visit(tree)
        struct.fields = meta_data.fields

    assert base_entity in entity_structs.keys(), "Couldn't find BaseEntity"

    # Add inherited fields to entities
    frozen_entity_structs = copy.deepcopy(entity_structs)  # If we don't deepcopy and freeze some types might be added twice
    for name, struct in frozen_entity_structs.items():
        parent = struct.parent
        struct = entity_structs[name]
        while parent:
            struct.fields = parent.fields + struct.fields
            parent = parent.parent

    def gen_fields_data(name, fields):
        def gen():
            out = []
            for field in fields:
                out.append(f"{{ typeid({field['TYPE']}), \"{field['NAME']}\", sizeof({field['TYPE']}), (int)offsetof({name}, {field['NAME']}) }}")
            return ",\n    ".join(out)
        return f"Field gen_{name}[] = {{\n    {gen()}\n}};"

    def gen_fields_switch(names):
        out = []
        for name in names:
            out.append(f"{' '*4}case EntityType::{to_enum(name)}: return {{ LEN(gen_{name}), gen_{name} }};")
        return "\n".join(out)

    fields_data = [gen_fields_data(name, struct.fields) for name, struct in entity_structs.items()]
    fields_switch = gen_fields_switch(entity_structs.keys())

    with open("tools/entity_types_type_of.h", "r") as template_file:
        template_type_of_h = Template(template_file.read())

    type_fields = { name: "\n".join([f"    {field['TYPE']} {field['NAME']}"
                                     + (f" = {field['INITIALIZER_VALUE']}" if 'INITIALIZER_VALUE' in field else "")
                                     + ";"
                                     for field in struct.fields])
                    for name, struct in entity_structs.items() }

    template_kwords_h = {
            "types": "\n".join([f"    {to_enum(t)}," for t in entity_structs.keys()]),
            "type_names": ",\n".join([f"{' '*4}\"{t}\"" for t in entity_structs.keys()]),
            "type_ofs": "\n".join([template_type_of_h.substitute(entity_type=t) for t in entity_structs.keys()]),
            "event_entity_bytes_union": "\n".join([f"{' '*8}u8 {to_enum(t)}[sizeof({t}) - sizeof(void *)];" for t in entity_structs.keys()]),
            "entity_events_prototypes": "\n".join([f"Event entity_event({name});" for name in entity_structs.keys()]),
    }

    with open("tools/entity_types.h", "r") as template_file:
        template = Template(template_file.read())
        with open("src/entity/entity_types.h", "w") as out_file:
            out_file.write(template.substitute(template_kwords_h))

    with open("tools/entity_types_type_of.cpp", "r") as template_file:
        template_type_of_cpp = Template(template_file.read())

    with open("tools/entity_types_event_callback.cpp", "r") as template_file:
        template_event_callback = Template(template_file.read())

    with open("tools/entity_types_event.cpp", "r") as template_file:
        template_entity_events = Template(template_file.read())

    callbacks = [template_event_callback.substitute(entity_type_enum=to_enum(name),
                                                    entity_type=name)
                 for name, struct in entity_structs.items()]

    entity_events = [template_entity_events.substitute(entity_type=name,
                                                       entity_type_enum=to_enum(name))
                     for name in entity_structs.keys()]

    template_kwords_cpp = {
            "type_ofs": "\n".join([template_type_of_cpp.substitute(entity_type=t, entity_type_enum=to_enum(t)) for t in entity_structs.keys()]),
            "type_formats": "\n".join([f"{' '*4}case EntityType::{to_enum(t)}: return snprintf(buffer, size, \"{t}\");" for t in entity_structs.keys()]),
            "fields_data": "\n".join(fields_data),
            "fields_switch": fields_switch,
            "callbacks": "".join(callbacks),
            "entity_events": "\n".join(entity_events),
    }

    with open("tools/entity_types.cpp", "r") as template_file:
        template = Template(template_file.read())
        with open("src/entity/entity_types.cpp", "w") as out_file:
            out_file.write(template.substitute(template_kwords_cpp))
