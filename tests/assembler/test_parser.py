from io import StringIO
from typing import Any, Iterable, Tuple
from unittest import TestCase, skip

from parameterized import parameterized

from ic4.assembler.expressions import Divide, Expression, Multiply, Subtract, Sum
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
                "long addition",
                "3+2+1+0+1+2+3",
                Sum(Sum(Sum(Sum(Sum(Sum(3, 2), 1), 0), 1), 2), 3),
            ),
            (
                "addiction and subtraction",
                "3-2+1+0-1+2+3",
                Sum(Sum(Subtract(Sum(Sum(Subtract(3, 2), 1), 0), 1), 2), 3),
            ),
            (
                "unitary minus",
                "-3",
                Multiply(3, -1),
            ),
            (
                "double unitary minus",
                "--3",
                Multiply(Multiply(3, -1), -1),
            ),
            (
                "sum to unitary minus",
                "1 + -3",
                Sum(1, Multiply(3, -1)),
            ),
            (
                "mult to unitary minus",
                "1 * -3",
                Multiply(1, Multiply(3, -1)),
            ),
            (
                "divide to unitary minus",
                "1 / -3",
                Divide(1, Multiply(3, -1)),
            ),
            (
                "divide from unitary minus",
                "-1 / 3",
                Divide(Multiply(1, -1), 3),
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
            (
                "Identifier",
                "a",
                "a",
            ),
            (
                "Identifier in math",
                "a+3",
                Sum("a", 3),
            ),
        ]
    )
    def test_parse(self, name: str, source: str, parsed: Iterable[Expression]):
        self.stream.write(source)
        self.stream.seek(0)

        self.assertEqual(self.parser.parse(self.lexer.tokenize(source)), parsed)
