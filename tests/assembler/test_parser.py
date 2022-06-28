from itertools import product
from typing import Iterable
from unittest import TestCase

from parameterized import parameterized

from ic4.assembler.commands import (
    Directive,
    DirectiveCode,
    Instruction,
    Label,
    OpCode,
    ParamMode,
)
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
            Instruction(OpCode["OUT"], ((ParamMode.ABSOLUTE, parsed),)),
        )

    @parameterized.expand(
        [
            (
                f"{opcode.name} {''.join({ParamMode.ABSOLUTE:'A',ParamMode.IMMEDIATE:'I', ParamMode.RELATIVE:'R'}[mode] for mode in parammodes)}",
                f"{opcode.name} {' '.join(f'{mode.prefix()}{param}' for mode, param in  zip(parammodes, names(opcode.param_number())))}",
                Instruction(
                    opcode, tuple(zip(parammodes, names(opcode.param_number())))
                ),
            )
            for opcode in OpCode
            for parammodes in product(ParamMode, repeat=opcode.param_number())
        ]
    )
    def test_parse_instructions(self, name: str, source: str, parsed: Instruction):
        self.assertEqual(
            self.parser.parse(self.lexer.tokenize(source + "\n"))[0], parsed
        )

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
                "single plus directive",
                "label : INTS 0\n",
                (Label("label"), Directive(DirectiveCode.INTS, (0,))),
            ),
            (
                "multiple plus directive",
                "label1 : label2 : label3 : INTS 0\n",
                (
                    Label("label1"),
                    Label("label2"),
                    Label("label3"),
                    Directive(DirectiveCode.INTS, (0,)),
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

    @parameterized.expand(
        [
            (
                "INTS",
                "INTS 0 1 2",
                Directive(DirectiveCode.INTS, (0, 1, 2)),
            ),
            (
                "INTS with expression",
                "INTS 0 1-2",
                Directive(DirectiveCode.INTS, (0, Subtract(1, 2))),
            ),
            (
                "INTS comma to separe parameters",
                "INTS 0 1 ,-2",
                Directive(DirectiveCode.INTS, (0, 1, Multiply(2, -1))),
            ),
            (
                "ZEROS",
                "ZEROS 32",
                Directive(DirectiveCode.ZEROS, (32,)),
            ),
            (
                "INC",
                "INC 4",
                Directive(DirectiveCode.INC, ((ParamMode.ABSOLUTE, 4),)),
            ),
            (
                "INC relative",
                "INC @4",
                Directive(DirectiveCode.INC, ((ParamMode.RELATIVE, 4),)),
            ),
            (
                "DEC",
                "DEC 4",
                Directive(DirectiveCode.DEC, ((ParamMode.ABSOLUTE, 4),)),
            ),
            (
                "DEC relative",
                "DEC @4",
                Directive(DirectiveCode.DEC, ((ParamMode.RELATIVE, 4),)),
            ),
            (
                "MOV",
                "MOV 4 @3",
                Directive(
                    DirectiveCode.MOV,
                    ((ParamMode.ABSOLUTE, 4), (ParamMode.RELATIVE, 3), 1),
                ),
            ),
            (
                "MOV multiple",
                "MOV 4 @3 15",
                Directive(
                    DirectiveCode.MOV,
                    ((ParamMode.ABSOLUTE, 4), (ParamMode.RELATIVE, 3), 15),
                ),
            ),
            (
                "LOAD",
                "LOAD 4 @3",
                Directive(
                    DirectiveCode.LOAD,
                    ((ParamMode.ABSOLUTE, 4), (ParamMode.RELATIVE, 3)),
                ),
            ),
            (
                "STORE",
                "STORE 4 @3",
                Directive(
                    DirectiveCode.STORE,
                    ((ParamMode.ABSOLUTE, 4), (ParamMode.RELATIVE, 3)),
                ),
            ),
            (
                "JMP",
                "JMP @3",
                Directive(
                    DirectiveCode.JMP,
                    ((ParamMode.RELATIVE, 3),),
                ),
            ),
            (
                "PUSH",
                "PUSH @3",
                Directive(
                    DirectiveCode.PUSH,
                    ((ParamMode.RELATIVE, 3), 1),
                ),
            ),
            (
                "PUSH multiple",
                "PUSH @3 15",
                Directive(
                    DirectiveCode.PUSH,
                    ((ParamMode.RELATIVE, 3), 15),
                ),
            ),
            (
                "PUSH only size",
                "PUSH , 15",
                Directive(
                    DirectiveCode.PUSH,
                    (None, 15),
                ),
            ),
            (
                "POP",
                "POP @3",
                Directive(
                    DirectiveCode.POP,
                    ((ParamMode.RELATIVE, 3), 1),
                ),
            ),
            (
                "POP multiple",
                "POP @3 15",
                Directive(
                    DirectiveCode.POP,
                    ((ParamMode.RELATIVE, 3), 15),
                ),
            ),
            (
                "POP only size",
                "POP , 15",
                Directive(
                    DirectiveCode.POP,
                    (None, 15),
                ),
            ),
        ]
    )
    def test_parse_directives(self, name: str, source: str, parsed: Instruction):
        self.assertEqual(
            self.parser.parse(self.lexer.tokenize(source + "\n"))[0], parsed
        )
