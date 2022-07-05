from itertools import product
from typing import Tuple
from unittest import TestCase, skip
from parameterized import parameterized
from ic4.assembler.assembler import Assembler, GenerateException

from ic4.assembly.commands import (
    DEC,
    INC,
    INTS,
    JMP,
    LOAD,
    MOV,
    STORE,
    ZEROS,
    Command,
    Directive,
    Instruction,
    Label,
    OpCode,
    Param,
    ParamMode,
)
from ic4.assembler.assembler import Assembler
from ic4.assembly.expressions import Constant, Reference
from ic4.assembly.srcfile import ExecutableHeader, SourceFile
from ic4.version import Version


def generate(source: Tuple[Command, ...]) -> Tuple[int, ...]:
    """Generate from a test body"""
    file = SourceFile(ExecutableHeader(Version(0, 1)), source)
    return Assembler(file).values


class TestGenerateInstruction(TestCase):
    @parameterized.expand(
        [
            (
                f"{opcode.name} {''.join({ParamMode.ABSOLUTE:'A',ParamMode.IMMEDIATE:'I', ParamMode.RELATIVE:'R'}[mode] for mode in parammodes)}",
                Instruction(
                    opcode,
                    tuple(
                        Param(m, Constant(v))
                        for m, v in zip(parammodes, range(opcode.param_number))
                    ),
                ),
                (
                    (
                        (
                            (
                                (
                                    (
                                        int(parammodes[2])
                                        if opcode.param_number > 2
                                        else 0
                                    )
                                    * 10
                                    + int(parammodes[1])
                                )
                                if opcode.param_number > 1
                                else 0
                            )
                            * 10
                            + int(parammodes[0])
                        )
                        if opcode.param_number > 0
                        else 0
                    )
                    * 100
                    + int(opcode),
                    *range(opcode.param_number),
                ),
            )
            for opcode in OpCode
            for parammodes in product(ParamMode, repeat=opcode.param_number)
        ]
    )
    def test_instruction(self, name: str, code: Command, output: Tuple[int, ...]):
        self.assertTupleEqual(generate((code,)), output)


class TestGenerateDirective(TestCase):
    def test_INTS(self):
        self.assertTupleEqual(
            generate((INTS(tuple(Constant(x) for x in (1, 1, 2, 3, 5, 8, 13))),)),
            (1, 1, 2, 3, 5, 8, 13),
        )

    def test_ZEROS(self):
        for i in (0, 10, 1000):
            with self.subTest(i):
                self.assertTupleEqual(
                    generate((ZEROS(Constant(i)),)),
                    tuple([0] * i),
                )

    def test_ZEROS_raise_incomplete(self):
        self.assertRaises(
            GenerateException,
            generate,
            (
                Label("a"),
                ZEROS(Reference("b")),
            ),
        )

    def test_ZEROS_raise_negative(self):
        self.assertRaises(
            GenerateException,
            generate,
            (
                Label("a"),
                ZEROS(Constant(-1)),
            ),
        )

    def test_DEC(self):
        self.assertTupleEqual(
            generate(
                (
                    DEC(
                        Param(ParamMode.ABSOLUTE, Constant(42)),
                    ),
                )
            ),
            generate(
                (
                    Instruction(
                        OpCode.ADD,
                        (
                            Param(ParamMode.ABSOLUTE, Constant(42)),
                            Param(ParamMode.IMMEDIATE, Constant(-1)),
                            Param(ParamMode.ABSOLUTE, Constant(42)),
                        ),
                    ),
                )
            ),
        )

    def test_INC(self):
        self.assertTupleEqual(
            generate(
                (
                    INC(
                        Param(ParamMode.ABSOLUTE, Constant(42)),
                    ),
                )
            ),
            generate(
                (
                    Instruction(
                        OpCode.ADD,
                        (
                            Param(ParamMode.ABSOLUTE, Constant(42)),
                            Param(ParamMode.IMMEDIATE, Constant(1)),
                            Param(ParamMode.ABSOLUTE, Constant(42)),
                        ),
                    ),
                )
            ),
        )

    def test_MOV(self):
        self.assertTupleEqual(
            generate(
                (
                    MOV(
                        Param(ParamMode.ABSOLUTE, Constant(42)),
                        Param(ParamMode.ABSOLUTE, Constant(76)),
                    ),
                )
            ),
            generate(
                (
                    Instruction(
                        OpCode.ADD,
                        (
                            Param(ParamMode.ABSOLUTE, Constant(42)),
                            Param(ParamMode.IMMEDIATE, Constant(0)),
                            Param(ParamMode.ABSOLUTE, Constant(76)),
                        ),
                    ),
                )
            ),
        )

    def test_MOV_multiple(self):
        for i in (0, 2, 10):  # number of places to move
            with self.subTest(i):
                self.assertTupleEqual(
                    generate(
                        (
                            MOV(
                                Param(ParamMode.ABSOLUTE, Constant(42)),
                                Param(ParamMode.ABSOLUTE, Constant(76)),
                                Constant(i),
                            ),
                        )
                    ),
                    generate(
                        tuple(
                            Instruction(
                                OpCode.ADD,
                                (
                                    Param(ParamMode.ABSOLUTE, Constant(42) + offset),
                                    Param(ParamMode.IMMEDIATE, Constant(0)),
                                    Param(ParamMode.ABSOLUTE, Constant(76) + offset),
                                ),
                            )
                            for offset in range(i)
                        )
                    ),
                )

    def test_LOAD(self):
        LOAD_code = generate(
            (
                LOAD(
                    Param(ParamMode.ABSOLUTE, Constant(42)),
                    Param(ParamMode.ABSOLUTE, Constant(76)),
                ),
            )
        )
        MOV_code = generate(
            (
                MOV(
                    Param(ParamMode.ABSOLUTE, Constant(42)),
                    Param(ParamMode.ABSOLUTE, Constant(12345)),
                ),
                MOV(
                    Param(ParamMode.ABSOLUTE, Constant(54321)),
                    Param(ParamMode.ABSOLUTE, Constant(76)),
                ),
            ),
        )
        # recovering the placeholders
        address_pos = MOV_code.index(12345)
        address_dest = MOV_code.index(54321)
        # patching mov code
        MOV_code = list(MOV_code)
        MOV_code[address_pos] = address_dest
        # the exact value of MOV_code[address_dest] is unimportant
        MOV_code[address_dest] = LOAD_code[address_dest]
        MOV_code = tuple(MOV_code)
        # checking equality
        self.assertTupleEqual(MOV_code, LOAD_code)
        # checking the placeholder is negative
        self.assertLess(LOAD_code[address_dest], 0, "Placeholder should be negative")

    def test_STORE(self):
        LOAD_code = generate(
            (
                STORE(
                    Param(ParamMode.ABSOLUTE, Constant(42)),
                    Param(ParamMode.ABSOLUTE, Constant(76)),
                ),
            )
        )
        MOV_code = generate(
            (
                MOV(
                    Param(ParamMode.ABSOLUTE, Constant(76)),
                    Param(ParamMode.ABSOLUTE, Constant(12345)),
                ),
                MOV(
                    Param(ParamMode.ABSOLUTE, Constant(42)),
                    Param(ParamMode.ABSOLUTE, Constant(54321)),
                ),
            ),
        )
        # recovering the placeholders
        address_pos = MOV_code.index(12345)
        address_dest = MOV_code.index(54321)
        # patching mov code
        MOV_code = list(MOV_code)
        MOV_code[address_pos] = address_dest
        # the exact value of MOV_code[address_dest] is unimportant
        MOV_code[address_dest] = LOAD_code[address_dest]
        MOV_code = tuple(MOV_code)
        # checking equality
        self.assertTupleEqual(MOV_code, LOAD_code)
        # checking the placeholder is negative
        self.assertLess(LOAD_code[address_dest], 0, "Placeholder should be negative")

    def test_JMP(self):
        self.assertTupleEqual(
            generate((JMP(Param(ParamMode.ABSOLUTE, Constant(42))),)),
            generate(
                (
                    Instruction(
                        OpCode.JNZ,
                        (
                            Param(ParamMode.IMMEDIATE, Constant(1)),
                            Param(ParamMode.ABSOLUTE, Constant(42)),
                        ),
                    ),
                )
            ),
        )


class TestAssemblerMethods(TestCase):
    """Check that the assembler has methods for all the directives"""

    @parameterized.expand(
        ((f"gen_{subclass.__name__}",) for subclass in Directive.__subclasses__())
    )
    def test_method(self, name: str):
        try:
            getattr(Assembler, name)
        except AttributeError:
            self.fail("Method is missing")
