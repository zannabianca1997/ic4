from itertools import combinations, combinations_with_replacement, product
import opcode
from typing import Iterable
from unittest import TestCase

from parameterized import parameterized

from ic4.assembler.commands import Instruction, Label, OpCode, ParamMode
from ic4.assembler.expressions import Divide, Expression, Multiply, Subtract, Sum
from ic4.assembler.lexer import ICAssLexer
from ic4.assembler.parser import ICAssParser


def names(n: int) -> Iterable[str]:
    return tuple(f"t_{i}" for i in range(n))


class TestParsing(TestCase):
    def setUp(self) -> None:
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
    def test_parse_expression(self, name: str, source: str, parsed: Expression):
        self.assertEqual(
            self.parser.parse(self.lexer.tokenize(f"OUT {source}\n"))[0],
            Instruction(OpCode["OUT"], ((ParamMode.MODE0, parsed),)),
        )

    @parameterized.expand(
        [
            (
                f"{opcode.name} {''.join({ParamMode.MODE0:'A',ParamMode.MODE1:'I', ParamMode.MODE2:'R'}[mode] for mode in parammodes)}",
                f"{opcode.name} {' '.join(f'{mode.prefix()}{param}' for mode, param in  zip(parammodes, names(opcode.param_number())))}\n",
                Instruction(
                    opcode, tuple(zip(parammodes, names(opcode.param_number())))
                ),
            )
            for opcode in OpCode
            for parammodes in product(ParamMode, repeat=opcode.param_number())
        ]
    )
    def test_parse_instructions(self, name: str, source: str, parsed: Instruction):
        self.assertEqual(self.parser.parse(self.lexer.tokenize(source))[0], parsed)

    @parameterized.expand(
        [
            (
                "single",
                "label : HALT\n",
                (Label("label"), Instruction(OpCode.HALT, ())),
            ),
            (
                "multiple",
                "label1 : label2 : label3 : HALT\n",
                (
                    Label("label1"),
                    Label("label2"),
                    Label("label3"),
                    Instruction(OpCode.HALT, ()),
                ),
            ),
            (
                "on different line",
                "label1 : \n HALT\n",
                (
                    Label("label1"),
                    Instruction(OpCode.HALT, ()),
                ),
            ),
        ]
    )
    def test_parse_labels(self, name: str, source: str, parsed: Instruction):
        self.assertTupleEqual(self.parser.parse(self.lexer.tokenize(source)), parsed)
