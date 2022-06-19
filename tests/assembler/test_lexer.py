from io import StringIO
from itertools import chain
from random import shuffle
from typing import Any, Iterable, Tuple
from unittest import TestCase
from ply.lex import Lexer, LexToken
from parameterized import parameterized
from ic4.assembler.commands import DirectiveCode, OpCode
from ic4.assembler.lexer import ICAssLexer


def tuplify(token: LexToken):
    return token.type, token.value, token.lineno, token.lexpos


class TestLexing(TestCase):
    stream: StringIO
    lexer: Lexer

    def setUp(self) -> None:
        self.stream = StringIO()
        self.lexer = ICAssLexer(self.stream, debug=True)

    @parameterized.expand([
        ("simple addition", "3+2", (
            ('NUMBER', 3, 1, 0),
            ('PLUS', '+', 1, 1),
            ('NUMBER', 2, 1, 2),
            ('LINE_END', None, 1, 3))),
        ("4 operations", "3+ 2 *6 /24", (
            ('NUMBER', 3, 1, 0),
            ('PLUS', '+', 1, 1),
            ('NUMBER', 2, 1, 3),
            ('TIMES', '*', 1, 5),
            ('NUMBER', 6, 1, 6),
            ('DIVIDE', '/', 1, 8),
            ('NUMBER', 24, 1, 9),
            ('LINE_END', None, 1, 11))),
        ("Punctuators", "@#,:", (
            ('RELATIVE', '@', 1, 0),
            ('IMMEDIATE', '#', 1, 1),
            ('COMMA', ',', 1, 2),
            ('COLON', ':', 1, 3),
            ('LINE_END', None, 1, 4))),
        ("Comments", """This is some code ; with comments
            But this is outside ; of the comments""", (
            ('IDENTIFIER', 'This', 1, 0),
            ('IDENTIFIER', 'is', 1, 5),
            ('IDENTIFIER', 'some', 1, 8),
            ('IDENTIFIER', 'code', 1, 13),
            ('LINE_END', None, 1, 33),
            ('IDENTIFIER', 'But', 2, 46),
            ('IDENTIFIER', 'this', 2, 50),
            ('IDENTIFIER', 'is', 2, 55),
            ('IDENTIFIER', 'outside', 2, 58),
            ('LINE_END', None, 2, 83))),
        ("Newlines", "\n\nINTS 0 ; one line interesting\n\n; this is filled even!\n\nADD 3 2 1\n\n;hello comments", (
            ('KEYWORD', 'INTS', 3, 2),
            ('NUMBER', 0, 3, 7),
            ('LINE_END', None, 3, 31),
            ('KEYWORD', 'ADD', 7, 57),
            ('NUMBER', 3, 7, 61),
            ('NUMBER', 2, 7, 63),
            ('NUMBER', 1, 7, 65),
            ('LINE_END', None, 7, 66)))
    ])
    def test_lex(self, name: str, source: str, lexed: Iterable[Tuple[str, Any, int, int]]):
        self.stream.write(source)
        self.stream.seek(0)

        self.assertTupleEqual(
            tuple(tuplify(tok) for tok in self.lexer),
            tuple(lexed)
        )

    def test_keywords(self):
        words = "Some names to distinguish from the keywords".split()
        keywords = set(x.name for x in chain(OpCode, DirectiveCode))
        if keywords.intersection(words):
            self.skipTest(f"{keywords.intersection(words)} are keywords now!")
        words += list(keywords)
        shuffle(words)
        self.stream.write(' '.join(words))
        self.stream.seek(0)

        self.assertTupleEqual(
            tuple((tok.value, tok.type) for tok in self.lexer),
            tuple([(word, 'KEYWORD' if (word in keywords) else 'IDENTIFIER')
                  for word in words] + [(None, 'LINE_END')])
        )
