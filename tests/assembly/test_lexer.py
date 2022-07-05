from itertools import chain
from random import shuffle
from typing import Any, Iterable, Tuple
from unittest import TestCase

from parameterized import parameterized
from sly.lex import Token, Lexer

from ic4.assembly.commands import OpCode, Directive
from ic4.assembly.lexer import ICAssLexer
from ic4.version import Version


def tuplify(token: Token):
    return token.type, token.value, token.lineno, token.index


DIRECTIVES = {s.__name__ for s in Directive.__subclasses__()}


class TestLexing(TestCase):
    lexer: Lexer

    def setUp(self) -> None:
        self.lexer = ICAssLexer()

    @parameterized.expand(
        [
            (
                "decimal number",
                "323",
                (("NUMBER", 323, 1, 0),),
            ),
            (
                "hexadecimal number",
                "0x323",
                (("NUMBER", 0x323, 1, 0),),
            ),
            (
                "char constants",
                r"'a' ' ' '\\' '\x12' '\10'",
                (
                    ("NUMBER", ord("a"), 1, 0),
                    ("NUMBER", ord(" "), 1, 4),
                    ("NUMBER", ord("\\"), 1, 8),
                    ("NUMBER", int("12", 16), 1, 13),
                    ("NUMBER", int("10", 8), 1, 20),
                ),
            ),
            (
                "simple addition",
                "3+2",
                (
                    ("NUMBER", 3, 1, 0),
                    ("PLUS", "+", 1, 1),
                    ("NUMBER", 2, 1, 2),
                ),
            ),
            (
                "4 operations",
                "3+ 2 *6 /24",
                (
                    ("NUMBER", 3, 1, 0),
                    ("PLUS", "+", 1, 1),
                    ("NUMBER", 2, 1, 3),
                    ("TIMES", "*", 1, 5),
                    ("NUMBER", 6, 1, 6),
                    ("DIVIDE", "/", 1, 8),
                    ("NUMBER", 24, 1, 9),
                ),
            ),
            (
                "Punctuators",
                "@#,:",
                (
                    ("RELATIVE", "@", 1, 0),
                    ("IMMEDIATE", "#", 1, 1),
                    ("COMMA", ",", 1, 2),
                    ("COLON", ":", 1, 3),
                ),
            ),
            (
                "Comments",
                """This is some code ; with comments
            But this is outside ; of the comments""",
                (
                    ("IDENTIFIER", "This", 1, 0),
                    ("IDENTIFIER", "is", 1, 5),
                    ("IDENTIFIER", "some", 1, 8),
                    ("IDENTIFIER", "code", 1, 13),
                    ("LINE_END", "\n", 1, 33),
                    ("IDENTIFIER", "But", 2, 46),
                    ("IDENTIFIER", "this", 2, 50),
                    ("IDENTIFIER", "is", 2, 55),
                    ("IDENTIFIER", "outside", 2, 58),
                ),
            ),
            (
                "Newlines",
                "\n\nINTS 0 ; one line interesting\n\n; this is filled even!\n\nADD 3 2 1\n\n;hello comments",
                (
                    ("LINE_END", "\n", 1, 0),
                    ("LINE_END", "\n", 2, 1),
                    ("INTS", "INTS", 3, 2),
                    ("NUMBER", 0, 3, 7),
                    ("LINE_END", "\n", 3, 31),
                    ("LINE_END", "\n", 4, 32),
                    ("LINE_END", "\n", 5, 55),
                    ("LINE_END", "\n", 6, 56),
                    ("OPCODE", "ADD", 7, 57),
                    ("NUMBER", 3, 7, 61),
                    ("NUMBER", 2, 7, 63),
                    ("NUMBER", 1, 7, 65),
                    ("LINE_END", "\n", 7, 66),
                    ("LINE_END", "\n", 8, 67),
                ),
            ),
            (
                "Version complete",
                "1.2.3_alpha",
                (("VERSION", Version(1, 2, 3, "alpha"), 1, 0),),
            ),
            ("Version no extra", "1.2.3", (("VERSION", Version(1, 2, 3), 1, 0),)),
            ("Version short", "1.2", (("VERSION", Version(1, 2), 1, 0),)),
            (
                "Version short with extra",
                "1.2_alpha",
                (("VERSION", Version(1, 2, 0, "alpha"), 1, 0),),
            ),
            (
                "Version wrong",
                "1_alpha",
                (
                    ("NUMBER", 1, 1, 0),
                    ("IDENTIFIER", "_alpha", 1, 1),
                ),
            ),
        ]
    )
    def test_lex(
        self, name: str, source: str, lexed: Iterable[Tuple[str, Any, int, int]]
    ):
        self.assertTupleEqual(
            tuple(tuplify(tok) for tok in self.lexer.tokenize(source)), tuple(lexed)
        )

    def test_keywords(self):
        words = "Some names to distinguish from the keywords _and some_identifier_012".split()
        keywords = set(chain((x.name for x in OpCode), DIRECTIVES))
        if keywords.intersection(words):
            self.skipTest(f"{keywords.intersection(words)} are keywords now!")
        words += list(keywords)
        shuffle(words)

        self.assertTupleEqual(
            tuple(
                (tok.value, tok.type) for tok in self.lexer.tokenize(" ".join(words))
            ),
            tuple(
                (
                    word,
                    "OPCODE"
                    if (word in {x.name for x in OpCode})
                    else (word if (word in DIRECTIVES) else "IDENTIFIER"),
                )
                for word in words
            ),
        )
