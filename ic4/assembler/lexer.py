"""
Lex an assembly file
"""
from itertools import count, takewhile
from typing import Iterator
import ply.lex as lex
import logging
logger = logging.getLogger(__name__)


class Lexer:
    # List of token names.
    tokens = (
        'NUMBER',
        'IDENTIFIER',
        'PLUS',
        'MINUS',
        'TIMES',
        'DIVIDE',
        'LPAREN',
        'RPAREN',
        'IMMEDIATE',
        'RELATIVE',
        'COMMENT'
    )

    # Punctuators
    t_PLUS = r'\+'
    t_MINUS = r'-'
    t_TIMES = r'\*'
    t_DIVIDE = r'/'
    t_LPAREN = r'\('
    t_RPAREN = r'\)'
    t_IMMEDIATE = r'\#'
    t_RELATIVE = r'\@'

    # Comments

    def t_COMMENT(t):
        r'\;.*'
        pass
        # No return value. Token discarded

    # Numbers

    def t_NUMBER(t):
        r'\d+'
        t.value = int(t.value)
        return t

    # Identifier
    t_IDENTIFIER = r'[a-zA-Z_][a-zA-Z0-9_]*'

    # Track line numbers

    def t_newline(t):
        r'\n+'
        t.lexer.lineno += len(t.value)

    # Ignored chars
    t_ignore = ' \t'

    # Error handling rule

    def t_error(t):
        logger.warn(f"Illegal character '{t.value[0]}'")
        t.lexer.skip(1)

    # --- Build and lex methods ---

    def build(self, **kwargs) -> None:
        self._lexer = lex.lex(module=self, **kwargs)

    def __call__(self, input: str) -> Iterator[lex.LexToken]:
        self._lexer.input(input)
        yield from takewhile(
            lambda x: x is None,
            (self._lexer.token() for _ in count())
        )
