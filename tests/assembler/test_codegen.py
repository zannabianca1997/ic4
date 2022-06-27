from itertools import product
from typing import Tuple
from unittest import TestCase
from parameterized import parameterized

from ic4.assembler.commands import (
    Command,
    Directive,
    DirectiveCode,
    Instruction,
    Label,
    OpCode,
    ParamMode,
)
from ic4.assembler.codegen import GenerateException, generate


class TestGenerateInstruction(TestCase):
    @parameterized.expand(
        [
            (
                f"{opcode.name} {''.join({ParamMode.ABSOLUTE:'A',ParamMode.IMMEDIATE:'I', ParamMode.RELATIVE:'R'}[mode] for mode in parammodes)}",
                Instruction(
                    opcode, tuple(zip(parammodes, range(opcode.param_number())))
                ),
                (
                    (
                        (
                            (
                                (
                                    (
                                        int(parammodes[2])
                                        if opcode.param_number() > 2
                                        else 0
                                    )
                                    * 10
                                    + int(parammodes[1])
                                )
                                if opcode.param_number() > 1
                                else 0
                            )
                            * 10
                            + int(parammodes[0])
                        )
                        if opcode.param_number() > 0
                        else 0
                    )
                    * 100
                    + int(opcode),
                    *range(opcode.param_number()),
                ),
            )
            for opcode in OpCode
            for parammodes in product(ParamMode, repeat=opcode.param_number())
        ]
    )
    def test_instruction(self, name: str, code: Command, output: Tuple[int, ...]):
        self.assertTupleEqual(generate((code,)), output)


class TestGenerate(TestCase):
    def test_INTS(self):
        self.assertTupleEqual(
            generate((Directive(DirectiveCode.INTS, (1, 1, 2, 3, 5, 8, 13)),)),
            (1, 1, 2, 3, 5, 8, 13),
        )

    def test_ZEROS(self):
        for i in (0, 10, 1000):
            with self.subTest(i):
                self.assertTupleEqual(
                    generate((Directive(DirectiveCode.ZEROS, (i,)),)),
                    tuple([0] * i),
                )

    def test_ZEROS_raise_incomplete(self):
        self.assertRaises(
            GenerateException,
            generate,
            (
                Label("a"),
                Directive(DirectiveCode.ZEROS, ("b",)),
            ),
        )

    def test_ZEROS_raise_negative(self):
        self.assertRaises(
            GenerateException,
            generate,
            (
                Label("a"),
                Directive(DirectiveCode.ZEROS, (-1,)),
            ),
        )

    def test_DEC(self):
        self.assertTupleEqual(
            generate((Directive(DirectiveCode.DEC, ((ParamMode.ABSOLUTE, 42),)),)),
            generate(
                (
                    Instruction(
                        OpCode.ADD,
                        (
                            (ParamMode.ABSOLUTE, 42),
                            (ParamMode.IMMEDIATE, -1),
                            (ParamMode.ABSOLUTE, 42),
                        ),
                    ),
                )
            ),
        )

    def test_INC(self):
        self.assertTupleEqual(
            generate((Directive(DirectiveCode.INC, ((ParamMode.ABSOLUTE, 42),)),)),
            generate(
                (
                    Instruction(
                        OpCode.ADD,
                        (
                            (ParamMode.ABSOLUTE, 42),
                            (ParamMode.IMMEDIATE, 1),
                            (ParamMode.ABSOLUTE, 42),
                        ),
                    ),
                )
            ),
        )
