"""
Lex an assembly file
"""
from itertools import count, takewhile
from typing import Iterator
import ply.lex as lex
import logging
logger = logging.getLogger(__name__)


class Lexer:
    # List of token names.   This is always required
    tokens = (
        'COMMENT',
        'NUMBER',
        'IDENTIFIER',
        'PLUS',
        'MINUS',
        'TIMES',
        'DIVIDE',
        'LPAREN',
        'RPAREN',
        'IMMEDIATE',
        'RELATIVE'
    )

    def t_COMMENT(self, t):
        r';.*'
        pass

    # Identifiers
    t_IDENTIFIER = r'[a-zA-Z_][a-zA-Z0-9_]*'

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
