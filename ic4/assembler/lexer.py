"""
Lex an assembly file
"""
from .commands import DirectiveCode, OpCode
import ply.lex as lex
from itertools import chain, count, takewhile
from typing import Iterator
import logging
logger = logging.getLogger(__name__)


class Lexer:
    # List of token names.   This is always required
    tokens = (
        'COMMENT',
        'NUMBER',
        'KEYWORD',
        'IDENTIFIER',
        'PLUS',
        'MINUS',
        'TIMES',
        'DIVIDE',
        'LPAREN',
        'RPAREN',
        'IMMEDIATE',
        'RELATIVE',
        'newline'
    )

    def t_COMMENT(self, t):
        r';.*'
        pass

    # Identifiers and keyword (note the order)
    @lex.TOKEN(" | ".join(x.name for x in chain(OpCode, DirectiveCode)))
    def t_KEYWORD(self, t):
        return t

    def t_IDENTIFIER(self, t):
        r'[a-zA-Z_][a-zA-Z0-9_]*'
        return t

    # Punctuators
    t_PLUS = r'\+'
    t_MINUS = r'-'
    t_TIMES = r'\*'
    t_DIVIDE = r'/'
    t_LPAREN = r'\('
    t_RPAREN = r'\)'
    t_IMMEDIATE = r'\#'
    t_RELATIVE = r'\@'

    # Numbers
    def t_NUMBER(self, t):
        r'\d+'
        t.value = int(t.value)
        return t

    # Define a rule so we can track line numbers
    def t_newline(self, t):
        r'\n+'
        t.lexer.lineno += len(t.value)
        return t

    # A string containing ignored characters (spaces and tabs)
    t_ignore = ' \t'

    # Error handling rule
    def t_error(self, t):
        logger.warn(f"Illegal character '{t.value[0]}'")
        t.lexer.skip(1)

    # --- Build and lex methods ---

    def __init__(self, **kwargs):
        self._lexer = lex.lex(module=self, **kwargs)

    def __call__(self, input: str) -> Iterator[lex.LexToken]:
        self._lexer.input(input)
        yield from takewhile(
            lambda x: x is not None,
            (self._lexer.token() for _ in count())
        )
