# TODO(ed): Don't write static colors, add a style instead.
TYPE_COLOR = "#f80"
TYPE_STRINGS = {
    "void",
    "f32", "f64", "real",
    "b8",
    "i8", "i16", "i32", "i64",
    "u8", "u16", "u32", "u64",
    "int", "bool", "char"
}

def is_type(string):
    global TYPE_STRINGS
    if string in TYPE_STRINGS:
        return True
    if string.strip("*&") == "":
        return True
    return string.strip("*&")[0].isupper() and any(map(lambda c: c.islower(), string))

KEYWORD_COLOR = "#99e"
KEYWORD_STRINGS = {
    "static", "struct", "class", "const", "public", "private", "override",
    "typename", "template", "enum"
}

def is_keyword(string):
    global KEYWORD_STRINGS
    return string in KEYWORD_STRINGS

MACRO_COLOR = "#0cc"
MACRO_STRINGS = {
    "LOG_MSG", "CLAMP", "LERP", "ABS", "ABS_MAX", "MAX", "SIGN",
    "SIGN_NO_ZERO", "ABS_MIN", "MIN", "IN_RANGE", "SQ", "MOD"
}

def is_macro(string):
    global MACRO_STRINGS
    if ":" in string:
        return False
    if string and string[0] == "#":
        return True
    if string in MACRO_STRINGS:
        return True
    return all(map(lambda c: c.isupper() or c == "_", string))

def is_constant(string):
    last = string.split(":")[-1]
    return all(map(lambda c: c.isupper() or c == "_", last))

COMMENT_COLOR="#999"
SPLIT_SEPS = set(" ,;:()<>=\n")

def highlight_code(line):
    """ Adds orange color to words marked to be highlighted """
    def highlight(word):
        styler = [
            (is_keyword, KEYWORD_COLOR),
            (is_macro, MACRO_COLOR),
            (is_type, TYPE_COLOR),
            (is_number, NUMBER_COLOR),
            (is_bool, BOOL_COLOR),
            (is_string, STRING_COLOR),
        ]

        for f, c in styler:
            if f(word):
                return color_html(word, c)

        return word

    if line.strip().startswith("//"):
        return color_html(line, COMMENT_COLOR)
    return "".join([highlight(w) + s for w, s in zip(*split_all(line, SPLIT_SEPS))])

def color_html(string, color):
    """ Wrap a string in a span with color specified as style. """
    return '<span style="color: {}">{}</span>'.format(color, string)


def split_all(string, sep):
    """
    Split a string at all characters in sep.
    Return two lists:
        words - Everything not including characters in sep
        splits - Everything including characters in sep

    Example:
        assert split_all("[1, 2]", " ,") == (["[1", "2]"], [", ", ""])

    The following is always true:
        len(words) == len(splits)
        len(words) > 0
        string == "".join([w + s for w, s in zip(*split_all(string, sep))])

    :param string str: String to split
    :param sep str: Characters where string should be split
    """
    words, splits = [], []
    word, split = "", ""
    i = 0
    while i < len(string):
        if string[i] in sep:
            words.append(word)
            word = ""
            while i < len(string) and string[i] in sep:
                split += string[i]
                i += 1
            splits.append(split)
            split = ""
            if i == len(string): break
        word += string[i]
        i += 1

    if word or not words:
        words.append(word)
        splits.append("")

    return words, splits

STRING_COLOR = "#cc8"
def is_string(string):
    """ Return whether or not a string is a... string """
    return string.startswith("\"") and string.endswith("\"")

NUMBER_COLOR = "#c8c"
def is_number(string):
    """ Return whether or not a string is a number """
    if string.startswith("0x"):
        return all(s.lower() in "0123456789abcdef" for s in string[2:])
    if string.startswith("0b"):
        return all(s in "01" for s in string[2:])
    else:
        return string.lstrip("-").replace(".", "", 1).isdigit()

BOOL_COLOR = "#c8c"
def is_bool(string):
    return string in ["true", "false"]
