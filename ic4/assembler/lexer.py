"""
Lex an assembly file
"""
from .commands import DirectiveCode, OpCode
import ply.lex as lex
from itertools import chain, count, takewhile
from typing import Dict, Hashable, Iterator, TextIO, Tuple
import logging
logger = logging.getLogger(__name__)


def _build_ICAssLexer(build_options):
    """Get a unbuilt lexer"""
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
        'COMMA',
        'COLON',
        'newline'
    )

    def t_COMMENT(t):
        r';.*'
        pass

    # Identifiers and keyword (note the order)
    @lex.TOKEN(" | ".join(sorted((x.name for x in chain(OpCode, DirectiveCode)), key=lambda x: len(x), reverse=True)))
    def t_KEYWORD(t):
        return t

    def t_IDENTIFIER(t):
        r'[a-zA-Z_][a-zA-Z0-9_]*'
        return t

    # Punctuators
    t_PLUS = r'\+'
    t_MINUS = r'\-'
    t_TIMES = r'\*'
    t_DIVIDE = r'\/'
    t_LPAREN = r'\('
    t_RPAREN = r'\)'
    t_IMMEDIATE = r'\#'
    t_RELATIVE = r'\@'
    t_COMMA = r'\,'
    t_COLON = r'\:'

    # Numbers
    def t_NUMBER(t):
        r'\d+'
        t.value = int(t.value)
        return t

    # Define a rule so we can track line numbers
    def t_newline(t):
        r'\n+'
        t.lexer.lineno += len(t.value)
        return t

    # A string containing ignored characters (spaces and tabs)
    t_ignore = ' \t'

    # Error handling rule
    def t_error(t):
        logger.warn(f"Illegal character '{t.value[0]}'")
        t.lexer.skip(1)

    # EOF handling rule
    def t_eof(t):
        # Get more input
        more = t.lexer.source.read(t.lexer.chunk_size)
        if more:
            # go to the end of the line to avoid breaking tokens
            while more[-1] != '\n' and (ch := t.lexer.source.read(1)):
                more += ch
            t.lexer.input(more)
            return t.lexer.token()
        return None

    return lex.lex(**build_options)


_built_lexers: Dict[Tuple[Tuple[str, Hashable], ...], lex.Lexer] = {}


def ICAssLexer(source: TextIO, chunk_size: int = 1024, **build_options):
    """Reaturn a lexer that will read from the given source"""
    # check if we have this lexer
    hash_kwargs = tuple(sorted(build_options.items()))
    if hash_kwargs not in _built_lexers:
        logger.info("Buinding lexer with args {}", build_options)
        _built_lexers[hash_kwargs] = _build_ICAssLexer(build_options)
    lexer = _built_lexers[hash_kwargs].clone()  # clone the lexer

    # add state as needed
    lexer.source = source
    lexer.chunk_size = chunk_size

    return lexer
