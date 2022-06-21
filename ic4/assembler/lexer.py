"""
Lex an assembly file
"""
from sys import stderr
from types import MethodType
from itertools import chain
from typing import Dict, Hashable, TextIO, Tuple
import logging

logger = logging.getLogger(__name__)

from sly import Lexer

from .commands import DirectiveCode, OpCode


class ICAssLexer(Lexer):
    """Lex IntCode Assembly"""

    tokens = {
        COMMENT,
        NUMBER,
        KEYWORD,
        IDENTIFIER,
        PLUS,
        MINUS,
        TIMES,
        DIVIDE,
        LPAREN,
        RPAREN,
        IMMEDIATE,
        RELATIVE,
        COMMA,
        COLON,
        LINE_END,
    }

    ignore = " \t"
    ignore_comment = r"\;.*"

    IDENTIFIER = r"[a-zA-Z_][a-zA-Z0-9_]*"

    for keyword in chain(OpCode, DirectiveCode):
        IDENTIFIER[keyword.name] = KEYWORD

    # Punctuators
    PLUS = r"\+"
    MINUS = r"\-"
    TIMES = r"\*"
    DIVIDE = r"\/"
    LPAREN = r"\("
    RPAREN = r"\)"
    IMMEDIATE = r"\#"
    RELATIVE = r"\@"
    COMMA = r"\,"
    COLON = r"\:"

    @_(r"0x[0-9a-fA-F]+", r"\d+")
    def NUMBER(self, t):
        if t.value.startswith("0x"):
            t.value = int(t.value[2:], 16)
        else:
            t.value = int(t.value)
        return t

    # this will match end of line, with optional whitespaces
    @_(r"\n(?:[ \t]*(?:;.*?)?\n)*[ \t]*")
    def LINE_END(self, t):
        self.lineno += t.value.count("\n")
        t.index += t.value.index("\n")
        t.value = None
        return t
