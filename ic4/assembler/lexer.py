"""
Lex an assembly file
"""
import logging

logger = logging.getLogger(__name__)

from sly import Lexer

from .commands import DirectiveCode, OpCode


class ICAssLexer(Lexer):
    """Lex IntCode Assembly"""

    tokens = {
        # misc
        COMMENT,
        LINE_END,
        COMMA,
        # instructions
        OPCODE,
        IMMEDIATE,
        RELATIVE,
        # directives
        INTS,
        ZEROS,
        INC,
        DEC,
        MOV,
        # labels
        IDENTIFIER,
        COLON,
        # expressions
        NUMBER,
        PLUS,
        MINUS,
        TIMES,
        DIVIDE,
        LPAREN,
        RPAREN,
    }

    ignore = " \t"
    ignore_comment = r"\;.*"

    IDENTIFIER = r"[a-zA-Z_][a-zA-Z0-9_]*"

    for keyword in OpCode:
        IDENTIFIER[keyword.name] = OPCODE

    # directives
    IDENTIFIER["INTS"] = INTS
    IDENTIFIER["ZEROS"] = ZEROS
    IDENTIFIER["INC"] = INC
    IDENTIFIER["DEC"] = DEC
    IDENTIFIER["MOV"] = MOV

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

    # this will match end of line and file
    @_(r"\n")
    def LINE_END(self, t):
        self.lineno += 1
        return t
