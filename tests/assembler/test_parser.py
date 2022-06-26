from io import StringIO
from typing import Any, Iterable, Tuple
from unittest import TestCase, skip

from parameterized import parameterized

from ic4.assembler.expressions import Divide, Expression, Multiply, Sum
from ic4.assembler.lexer import ICAssLexer
from ic4.assembler.parser import ICAssParser


class TestParsingExpression(TestCase):
    def setUp(self) -> None:
        self.stream = StringIO()
        self.lexer = ICAssLexer()
        self.parser = ICAssParser()

    @parameterized.expand(
        [
            (
                "simple addition",
                "3+2",
                Sum(3, 2),
            ),
            (
                "4 operations",
                "3+ 2 *6 /24",
                Sum(3, Divide(Multiply(2, 6), 24)),
            ),
            (
                "Parenthesis",
                "(3+ 6) /24",
                Divide(Sum(3, 6), 24),
            ),
        ]
    )
    def test_parse(self, name: str, source: str, parsed: Iterable[Expression]):
        self.stream.write(source)
        self.stream.seek(0)

        self.assertEqual(self.parser.parse(self.lexer.tokenize(source)), parsed)
