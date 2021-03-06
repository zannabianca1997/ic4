from re import compile
from string import ascii_letters, digits, printable, punctuation
from typing import Iterable, Tuple

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


const_string_re = compile(
    r"\"(?:[^\"\\\n]|\\(?:[abfnrtv\\\'\"\?]|[xX][0-9a-fA-F]+|[0-7]{1,3}))*\""
)

_hex_re = compile(r"^[0-9a-fA-F]+")
_oct_re = compile(r"^[0-7]{1,3}")


def unescape_string_const(char_string: str) -> Tuple[int, ...]:
    assert const_string_re.fullmatch(char_string)
    char_string = char_string[1:-1]
    unescaped = []
    while char_string:
        # pop first char
        ch = char_string[0]
        char_string = char_string[1:]

        if ch != "\\":
            unescaped.append(ord(ch))
            continue

        # pop next char
        ch = char_string[0]
        char_string = char_string[1:]

        if ch in ESCAPE_DICT:
            unescaped.append(ord(ESCAPE_DICT[ch]))
            continue

        if ch == "x":
            m = _hex_re.match(char_string)
            char_string = char_string[m.end() :]
            unescaped.append(int(m[0], 16))
            continue

        char_string = ch + char_string
        m = _oct_re.match(char_string)
        char_string = char_string[m.end() :]
        unescaped.append(int(m[0], 8))

    return tuple(unescaped)


def escape_string_const(char_codes: Iterable[int]) -> str:
    return (
        '"'
        + "".join(
            (
                chr(x)
                if x
                in frozenset(ord(c) for c in digits + ascii_letters + punctuation + " ")
                else "\\" + oct(x)[2:].rjust(3, "0")
            )
            for x in char_codes
        )
        + '"'
    )
