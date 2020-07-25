#!/usr/bin/env python3

import glob
from lark import lark
import re
from string import Template


class Struct():
    """A struct or a class in C/C++.

    Can be used in a tree since it knows about its base.

    Members:
      - name   : str
      - parent : Struct
      - source : str
    """
    def __init__(self, name, parent=None):
        self.name = name
        self.parent = parent
        self.source = ""

    def parents_contain(self, name):
        """Return whether this struct/class inherits from `name`."""
        if self.parent is None:
            return False
        elif name == self.parent.name:
            return True
        else:
            return self.parent.parents_contain(name)

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


if __name__ == "__main__":
    lexer = lark.Lark.open("tools/struct.lark", start="structs")
    types = []
    base_entity = "BaseEntity"
    for name, struct in sorted(find_structs(glob.glob("src/**/*.*", recursive=True)).items()):
        if struct.parents_contain(base_entity) or name == base_entity:
            tree = lexer.parse(struct.source)
            struct_types = list(tree.find_data("struct_type"))
            if len(struct_types) == 0:
                print("No typename found, 1 required")
                continue
            if len(struct_types) > 1:
                print(f"{len(struct_types)} typenames found (structs in structs?), 1 required\nstructs are {struct_types}")
                continue
            types.append(struct_types[0].children[0])
    assert base_entity in types, "Couldn't find BaseEntity"

    with open("tools/entity_types_type_of.h", "r") as template_file:
        template_type_of_h = Template(template_file.read())

    template_kwords_h = {
            "types": "\n".join([f"    {t.upper()}," for t in types]),
            "type_ofs": "\n".join([template_type_of_h.substitute(entity_type=t) for t in types]),
    }

    with open("tools/entity_types.h", "r") as template_file:
        template = Template(template_file.read())
        with open("src/entity/entity_types.h", "w") as out_file:
            out_file.write(template.substitute(template_kwords_h))

    with open("tools/entity_types_type_of.cpp", "r") as template_file:
        template_type_of_cpp = Template(template_file.read())

    template_kwords_cpp = {
            "type_ofs": "\n".join([template_type_of_cpp.substitute(entity_type=t, entity_type_enum=t.upper()) for t in types]),
            "type_formats": "\n".join([f"{' '*8}case EntityType::{t.upper()}: return snprintf(buffer, size, \"{t}\");" for t in types]),
    }

    with open("tools/entity_types.cpp", "r") as template_file:
        template = Template(template_file.read())
        with open("src/entity/entity_types.cpp", "w") as out_file:
            out_file.write(template.substitute(template_kwords_cpp))
