#!/usr/bin/env python3
import re
from itertools import repeat
from glob import glob
from enum import Enum
from highlighter import highlight_code, split_all
from dataclasses import dataclass


# Used to describe what is in this file, the first line is also
# mined for a name.
HEADING = 1
DOC = 2  # API referend.
COMMENT = 3  # Misc comments.

@dataclass
class Comment:
    kind: int
    path: str
    line: int
    namespace: str = ""
    comment: str = ""


def find_comments(file_path):
    """
    Parses out the comments of a file, categorizing them and making stripping
    them of unnessecary whitespace.
    """
    with open(file_path) as f:
        appending_to_comment = False
        heading = ""
        comments = []
        comobj = Comment(None, file_path, 0)
        current_type = None
        bracket_stack = 0
        for linenr, line in enumerate(f):
            if not appending_to_comment:
                next_type = None
                if "namespace" in line and not comments:
                    words = line.split()
                    if words[0] == "namespace" and len(words) <= 3:
                        comobj.namespace = words[1]
                if "///#" in line:
                    next_type = HEADING
                    heading = line[4:].strip()
                elif "///*" in line:
                    next_type = DOC
                elif "////" in line:
                    next_type = COMMENT

                if next_type is not None:
                    appending_to_comment = True
                    if comobj.kind is not None:
                        comments.append(comobj)
                    comobj = Comment(next_type, file_path, linenr)
                    if comobj.kind != HEADING:
                        comobj.comment = line

            elif appending_to_comment:
                bracket_stack += line.count("{") - line.count("}")
                blank_line = line.strip() == ""
                if blank_line and bracket_stack <= 0:
                    appending_to_comment = False
                    bracket_stack = 0
                else:
                    comobj.comment += line
    if comobj.comment:
        comments.append(comobj)
    return heading, comments


def find_all_comments(files):
    """
    Finds all comments in a list of files and creates a documentation "tree"
    where the order of the modules are keept.
    Kinda structured like this:
        [modules { submodules: [comments] }]
    Making it easy to create the documentation table and structure the
    comments.
    """
    # TODO(ed): This can probably be a list comphrehension.
    regions = []
    for region, _ in files:
        if region not in regions:
            regions.append(region)
    documentation = {region: dict() for region in regions}
    for region, file_path in sorted(files):
        heading, comments = find_comments(file_path)
        if heading and comments:
            documentation[region][heading] = comments
        elif comments:
            documentation[region]["global"] = comments
    return [(region, documentation[region]) for region in reversed(regions)]


def tag(tag, content, html_class="", html_id=""):
    """
    Surrounds a piece of content in an html tag, with optional added
    class.
    """
    html_class = " class='{}'".format(html_class) if html_class else ""
    html_id = " id='{}'".format(html_id) if html_id else ""
    return "<{}{}{}>{}</{}>".format(tag, html_id, html_class, content, tag)


def link(text, to):
    """
    Creates a hyper link with the text to the position supplied.
    """
    return "<a href=\"{}\">{}</a>".format(to, text)


def make_id_friendly(string):
    """
    Returns the string but made into something that can be used
    as an ID.
    """
    return re.sub(r"[^a-z0-9]", "", string.lower())


def insert_links(line, docs):
    def link(l):
        if (match := l) in docs:
            return "<a href='{}' class='code-link'>{}</a>".format("#" + docs[match], match)
        elif (match := l.split("::")[-1]) in docs:
            return "::<a href='{}' class='code-link'>{}</a>".format("#" + docs[match], match)
        return l
    SPLIT_SEPS = set(" ,;(){}[]<>.-'*+\n")
    return "".join([link(w) + s for w, s in zip(*split_all(line, SPLIT_SEPS))])

def process_comment_section(lines, docs):
    """
    Parses out code from text and applies the appropriate markup.
    """
    out = "<p>"
    in_comment = True
    for line in lines:
        if line.strip() == "": continue
        if line in ("SMEK_EXPORT", "SMEK_EXPORT_STRUCT"): continue
        if not in_comment and line.startswith("//"):
            in_comment = True
            if out:
                out += "</p>\n"
            out += "<p>"
        if in_comment and not line.startswith("//"):
            in_comment = False
            out += "</p>\n"
            out += "<p class='code'>"
        if in_comment:
            if line == "//":
                to_append = " "
            else:
                to_append = " " + insert_links(line.replace("// ", "").strip(), docs)
            if to_append:
                out += to_append
            else:
                out += "</p><p>"
        else:
            indent = len(line) - len(line.lstrip())
            out += "<span indent=\"{}\"></span>".format("#" * indent)
            safe_code = line.replace("<", "&lt;").replace(">", "&gt;").lstrip()
            out += insert_links(highlight_code(safe_code), docs)
            out += "<br>\n"
    return out.replace("<p></p>", "").strip()


def find_comment_title(comment):
    """
    Finds the title of a comment.
    """
    return comment[5:comment.index("\n")].capitalize()


def find_comment_id(section, comment):
    return make_id_friendly(section + find_comment_title(comment))


def format_comment(section, comobj, docs):
    """
    Formats the code according to how a comment should be formatted.
    """
    comment = comobj.comment
    namespace = comobj.namespace
    title = find_comment_title(comment)
    return tag("div", tag("h3", title) + "\n" +
               tag("h5", comobj.path + ":" + str(comobj.line), "linenr") + \
               process_comment_section(comment.split("\n")[1:], docs) + "\n",
               "block comment",
               find_comment_id(section, comment)) + "\n"


def find_documentation_title(heading, comment, namespace):
    """
    Finds the title for this piece of documentation.
    """
    for line in comment.split("\n"):
        if "///*" in line:
            potential_title = line[5:].strip()
            if potential_title:
                return potential_title
        words = line.strip().split()
        line = " ".join(words) # make sure only single spaces exist
        if match := re.search(r"^[^/]*?(?:struct|class) (\S+)", line):
            return match[1]
        if not (line.startswith("//") or line.startswith("template")): # found line of code, assume it contains the title
            for word in words:
                if "(" in word:
                    return word[:word.index("(")].replace("*", "")
            return line[:re.search(r"[;=]", line).start()] # no function found, assume variable
    return "ERROR-NO-TITLE"


def find_documentation_id(section, comment):
    for line in comment.split("\n"):
        if "///*" in line:
            potential_title = line[5:].strip()
            if potential_title:
                return potential_title
        if not (line.startswith("//") or line.startswith("template")): # found line of code, assume it contains the title
            return make_id_friendly(line)
    return "ERROR-NO-ID"


def format_documentation(section, comobj, docs):
    """
    Formats the code according to how a comment should be formatted.
    """
    comment = comobj.comment
    namespace = comobj.namespace
    title = find_documentation_title(section, comment, namespace)
    if namespace:
        namespace = tag("span", namespace + "::", html_class="namespace")
    return tag("div", "\n" + tag("h3", namespace + title) + "\n" +
               tag("h5", comobj.path + ":" + str(comobj.line), "linenr") + \
               process_comment_section(comment.split("\n")[1:], docs) + "\n",
               "block doc",
               find_documentation_id(section, comment)) + "\n"


def format_heading(heading, comobj, _):
    comment = comobj.comment
    namespace = comobj.namespace
    return tag("h2", heading, "section heading", make_id_friendly(heading)) + \
           tag("h5", comobj.path + ":" + str(comobj.line), "linenr") + \
           tag("p", comment.replace("///#", "").replace("//", "").strip())


def has_content(region_headings):
    for heading in region_headings:
        for comobj in region_headings[heading]:
            if comobj.comment:
                return True
    return False


def write_documentation(path, documentation):
    with open(path, "w") as f:
        PREAMBLE = "<html>\n<head>\n<title>SMEK - Documentation</title>\n<meta charset=utf-8>\n<link rel='icon' type='image/png' href='favicon.png'>\n<script src=\"script.js\"></script>\n<link rel=\"stylesheet\" type=\"text/css\" href=\"style.css\">\n</head>\n\n<body>\n"
        f.write(PREAMBLE)
        # Writing nav
        documented_code = {}
        f.write("<nav>\n<div class='container'>\n<img id='logo' src='SMEK_logo.svg'>\n<h2>Content</h2>\n")
        f.write("<ul id=\"nav\">\n")
        for region, headings in sorted(documentation):
            if not has_content(headings): continue
            f.write(tag("li", link(region.capitalize(), "#" + region), "hide hideable"))
            f.write("\n<li><ul>\n")
            for heading in headings:
                f.write(tag("li", link(heading, "#" + make_id_friendly(heading)),  "hide hideable"))
                f.write("\n<li><ul>\n")
                for comobj in headings[heading]:
                    if not comobj.comment: continue

                    if comobj.kind == HEADING:
                        continue
                    elif comobj.kind == DOC:
                        text = find_documentation_title(heading, comobj.comment, comobj.namespace)
                        html_id = find_documentation_id(heading, comobj.comment)
                    elif comobj.kind == COMMENT:
                        text = find_comment_title(comobj.comment)
                        html_id = find_comment_id(heading, comobj.comment)
                    documented_code[text] = html_id
                    documented_code[text.split("::")[-1]] = html_id
                    f.write(tag("li", link(text, "#" + html_id)))
                f.write("\n</li></ul>\n")

            f.write("</li></ul>\n")
        f.write("</ul>\n")
        f.write("</div>\n</nav>\n\n")

        f.write("<article>\n")
        for region, headings in documentation:
            if not has_content(headings): continue
            f.write(tag("h1", region.capitalize(), "region heading", region) + "\n")
            for heading in headings:
                for comobj in headings[heading]:
                    if not comobj.comment: continue
                    args = (heading, comobj, documented_code)
                    # Formats the comments to a more suitable HTML format.
                    if comobj.kind == HEADING:
                        output = format_heading(*args)
                    elif comobj.kind == DOC:
                        output = format_documentation(*args)
                    elif comobj.kind == COMMENT:
                        output = format_comment(*args)
                    f.write(output)
        f.write("</article>\n")
        f.write("</body>\n</html>")


if __name__ == "__main__":
    def find_category(filename):
        splits = filename.split("/")
        if len(splits) < 3:
            return "core"
        return splits[-2]

    all_files = [(find_category(f), f) for f in glob("src/**/*.*", recursive=True)]
    valid_files = [(region, file) for (region, file) in all_files
                  if not re.match(r".*\.o$", file)]
    documentation = find_all_comments(valid_files)
    write_documentation("docs/index.html", documentation)
