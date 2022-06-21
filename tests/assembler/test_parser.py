from io import StringIO
from typing import Any, Iterable, Tuple
from unittest import TestCase, skip

from ply.yacc import LRParser
from ply.lex import Lexer
from parameterized import parameterized

from ic4.assembler.expressions import Divide, Expression, Multiply, Sum
from ic4.assembler.lexer import ICAssLexer
from ic4.assembler.parser import ICAssParser


class TestParsingExpression(TestCase):
    stream: StringIO
    lexer: Lexer
    parser: LRParser

    def setUp(self) -> None:
        self.stream = StringIO()
        self.lexer = ICAssLexer(self.stream, debug=True)
        self.parser = ICAssParser(debug=True, start="expression")

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
        ]
    )
    @skip("Parser not implemented")
    def test_parse(self, name: str, source: str, parsed: Iterable[Expression]):
        self.stream.write(source)
        self.stream.seek(0)

        self.assertEqual(self.parser.parse(lexer=self.lexer), parsed)
