from re import compile

ESCAPE_DICT = {
    "a": "\a",
    "b": "\b",
    "f": "\f",
    "n": "\n",
    "r": "\r",
    "t": "\t",
    "v": "\v",
    "\\": "\\",
    "'": "'",
    '"': '"',
    "?": "?",
}

char_const_re = compile(
    r"\'(?:[^\'\\\n]|\\(?:[abfnrtv\\\'\"\?]|[xX][0-9a-fA-F]+|[0-9]{1,3}))\'"
)


def unescape_char_const(char_const: str) -> int:
    assert char_const_re.fullmatch(char_const)
    if char_const[1] != "\\":
        return ord(char_const[1])
    if char_const[2] in ESCAPE_DICT:
        return ord(ESCAPE_DICT[char_const[2]])
    if char_const[2] == "x":
        return int(char_const[3:-1], 16)
    if char_const[2].isdigit():
        return int(char_const[2:-1], 8)
    raise ValueError(f"Invalid escape sequence {char_const[1:-1]}")
