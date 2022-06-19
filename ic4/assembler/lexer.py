"""
Lex an assembly file
"""
from .commands import DirectiveCode, OpCode
import ply.lex as lex
from itertools import chain, count, takewhile
from typing import Dict, Hashable, Iterator, TextIO, Tuple
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

    # EOF handling rule
    def t_eof(self, t):
        # Get more input (Example)
        more = self.source.read(self.chunk_size)
        if more:
            self._lexer.input(more)
            return self._lexer.token()
        return None

    # --- Build and lex methods ---

    _class_lexers: Dict[Tuple[Tuple[str, Hashable], ...], lex.Lexer] = {}
    _lexer: lex.Lexer

    def __init__(self, source: TextIO, chunk_size: int = 1024, **kwargs):
        # check if we have this lexer
        hash_kwargs = tuple(sorted(kwargs.items()))
        if hash_kwargs not in self._class_lexers:
            logger.info("Buinding lexer with args {}", kwargs)
            self._class_lexers[hash_kwargs] = lex.lex(module=self, **kwargs)
        self._lexer = self._class_lexers[hash_kwargs].clone()
        self.source = source
        self.chunk_size = chunk_size

    def __iter__(self) -> Iterator[lex.LexToken]:
        return self

    def __next__(self) -> lex.LexToken:
        return self._lexer.next()
