from itertools import product
from typing import Iterable, Tuple
from unittest import TestCase

from parameterized import parameterized

from ic4.assembly.commands import (
    CALL,
    DEC,
    INC,
    INTS,
    JMP,
    LOAD,
    MOV,
    POP,
    PUSH,
    RET,
    STORE,
    ZEROS,
    Directive,
    Instruction,
    Label,
    OpCode,
    Param,
    ParamMode,
)
from ic4.assembly.expressions import (
    Constant,
    Divide,
    Expression,
    Multiply,
    Reference,
    Subtract,
    Sum,
)
from ic4.assembly.lexer import ICAssLexer
from ic4.assembly.parser import ICAssParser
from ic4.assembly.srcfile import ExecutableHeader, ObjectsHeader
from ic4.version import Version


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
                Sum(Constant(3), Constant(2)),
            ),
            (
                "long addition",
                "3+2+1+0+1+2+3",
                Sum(
                    Sum(
                        Sum(
                            Sum(
                                Sum(Sum(Constant(3), Constant(2)), Constant(1)),
                                Constant(0),
                            ),
                            Constant(1),
                        ),
                        Constant(2),
                    ),
                    Constant(3),
                ),
            ),
            (
                "addiction and subtraction",
                "3-2+1+0-1+2+3",
                Sum(
                    Sum(
                        Subtract(
                            Sum(
                                Sum(Subtract(Constant(3), Constant(2)), Constant(1)),
                                Constant(0),
                            ),
                            Constant(1),
                        ),
                        Constant(2),
                    ),
                    Constant(3),
                ),
            ),
            (
                "unitary minus",
                "-3",
                Multiply(Constant(3), Constant(-1)),
            ),
            (
                "double unitary minus",
                "--3",
                Multiply(Multiply(Constant(3), Constant(-1)), Constant(-1)),
            ),
            (
                "sum to unitary minus",
                "1 + -3",
                Sum(Constant(1), Multiply(Constant(3), Constant(-1))),
            ),
            (
                "mult to unitary minus",
                "1 * -3",
                Multiply(Constant(1), Multiply(Constant(3), Constant(-1))),
            ),
            (
                "divide to unitary minus",
                "1 / -3",
                Divide(Constant(1), Multiply(Constant(3), Constant(-1))),
            ),
            (
                "divide from unitary minus",
                "-1 / 3",
                Divide(Multiply(Constant(1), Constant(-1)), Constant(3)),
            ),
            (
                "4 operations",
                "3+ 2 *6 /24",
                Sum(
                    Constant(3),
                    Divide(Multiply(Constant(2), Constant(6)), Constant(24)),
                ),
            ),
            (
                "Parenthesis",
                "(3+ 6) /24",
                Divide(Sum(Constant(3), Constant(6)), Constant(24)),
            ),
            (
                "Identifier",
                "a",
                Reference("a"),
            ),
            (
                "Identifier in math",
                "a+3",
                Sum(Reference("a"), Constant(3)),
            ),
        ]
    )
    def test_parse_expression(self, name: str, source: str, parsed: Expression):
        self.assertEqual(
            self.parser.parse(
                self.lexer.tokenize(f"EXECUTABLE 0.1\nOUT {source}\n")
            ).body[0],
            Instruction(OpCode.OUT, (Param(ParamMode.ABSOLUTE, parsed),)),
        )

    @parameterized.expand(
        [
            (
                f"{opcode.name} {''.join({ParamMode.ABSOLUTE:'A',ParamMode.IMMEDIATE:'I', ParamMode.RELATIVE:'R'}[mode] for mode in parammodes)}",
                f"{opcode.name} {' '.join(f'{mode.prefix}{param}' for mode, param in  zip(parammodes, names(opcode.param_number)))}",
                Instruction(
                    opcode,
                    tuple(
                        Param(p, Reference(n))
                        for p, n in zip(parammodes, names(opcode.param_number))
                    ),
                ),
                parammodes,
            )
            for opcode in OpCode
            for parammodes in product(ParamMode, repeat=opcode.param_number)
        ]
    )
    def test_parse_instructions(
        self,
        name: str,
        source: str,
        expected: Instruction,
        modes: Tuple[ParamMode, ...],
    ):
        parsed = self.parser.parse(
            self.lexer.tokenize("EXECUTABLE 0.1\n" + source + "\n")
        ).body[0]
        # check the throw
        if any(
            writ and mode == ParamMode.IMMEDIATE
            for writ, mode in zip(expected.opcode.writes_to, modes)
        ):
            self.assertRaises(AssertionError, parsed.check)
        else:
            parsed.check()  # should run
        self.assertEqual(parsed, expected)

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
                (Label("label"), INTS((Constant(0),))),
            ),
            (
                "multiple plus directive",
                "label1 : label2 : label3 : INTS 0\n",
                (
                    Label("label1"),
                    Label("label2"),
                    Label("label3"),
                    INTS((Constant(0),)),
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
        self.assertTupleEqual(
            self.parser.parse(self.lexer.tokenize("EXECUTABLE 0.1\n" + source)).body,
            parsed,
        )

    @parameterized.expand(
        [
            (
                "INTS",
                "INTS 0 1 2",
                INTS(tuple(Constant(x) for x in (0, 1, 2))),
            ),
            (
                "INTS with string",
                'INTS "This is a String, \\n with escapes!"',
                INTS(
                    tuple(
                        Constant(ord(x)) for x in "This is a String, \n with escapes!\0"
                    )
                ),
            ),
            (
                "INTS with expression",
                "INTS 0 1-2",
                INTS((Constant(0), Constant(1) - 2)),
            ),
            (
                "INTS comma to separe parameters",
                "INTS 0 1 ,-2",
                INTS((Constant(0), Constant(1), Constant(-1) * 2)),
            ),
            (
                "ZEROS",
                "ZEROS 32",
                ZEROS(Constant(32)),
            ),
            ("INC", "INC 4", INC(Param(ParamMode.ABSOLUTE, Constant(4)))),
            ("INC relative", "INC @4", INC(Param(ParamMode.RELATIVE, Constant(4)))),
            ("DEC", "DEC 4", DEC(Param(ParamMode.ABSOLUTE, Constant(4)))),
            ("DEC relative", "DEC @4", DEC(Param(ParamMode.RELATIVE, Constant(4)))),
            (
                "MOV",
                "MOV 4 @3",
                MOV(
                    Param(ParamMode.ABSOLUTE, Constant(4)),
                    Param(ParamMode.RELATIVE, Constant(3)),
                    Constant(1),
                ),
            ),
            (
                "MOV multiple",
                "MOV 4 @3 15",
                MOV(
                    Param(ParamMode.ABSOLUTE, Constant(4)),
                    Param(ParamMode.RELATIVE, Constant(3)),
                    Constant(15),
                ),
            ),
            (
                "LOAD",
                "LOAD 4 @3",
                LOAD(
                    Param(ParamMode.ABSOLUTE, Constant(4)),
                    Param(ParamMode.RELATIVE, Constant(3)),
                ),
            ),
            (
                "STORE",
                "STORE 4 @3",
                STORE(
                    Param(ParamMode.ABSOLUTE, Constant(4)),
                    Param(ParamMode.RELATIVE, Constant(3)),
                ),
            ),
            (
                "JMP",
                "JMP @3",
                JMP(
                    Param(ParamMode.RELATIVE, Constant(3)),
                ),
            ),
            (
                "PUSH",
                "PUSH @3",
                PUSH(Param(ParamMode.RELATIVE, Constant(3)), Constant(1)),
            ),
            (
                "PUSH multiple",
                "PUSH @3 15",
                PUSH(Param(ParamMode.RELATIVE, Constant(3)), Constant(15)),
            ),
            (
                "PUSH only size",
                "PUSH , 15",
                PUSH(None, Constant(15)),
            ),
            (
                "POP",
                "POP @3",
                POP(Param(ParamMode.RELATIVE, Constant(3)), Constant(1)),
            ),
            (
                "POP multiple",
                "POP @3 15",
                POP(Param(ParamMode.RELATIVE, Constant(3)), Constant(15)),
            ),
            (
                "POP only size",
                "POP , 15",
                POP(None, Constant(15)),
            ),
            (
                "CALL",
                "CALL #15",
                CALL(Param(ParamMode.IMMEDIATE, Constant(15))),
            ),
            ("RET", "RET", RET()),
        ]
    )
    def test_parse_directives(self, name: str, source: str, expected: Directive):
        parsed = self.parser.parse(
            self.lexer.tokenize("EXECUTABLE 0.1\n" + source + "\n")
        ).body[0]
        parsed.check()  # check parameters are ok
        self.assertEqual(parsed, expected)

    def test_parse_exec_header(self):
        parsed = self.parser.parse(
            self.lexer.tokenize("EXECUTABLE 1.2.3_test\n")
        ).header
        self.assertEqual(parsed, ExecutableHeader(Version(1, 2, 3, "test")))

    @parameterized.expand(
        [
            ("Plain", "OBJECTS 1.2.3_test\n", ObjectsHeader(Version(1, 2, 3, "test"))),
            (
                "with entry",
                "OBJECTS 3.4\nENTRY main\n",
                ObjectsHeader(Version(3, 4), entry="main"),
            ),
            (
                "with export and extern",
                "OBJECTS 3.4\nEXTERN a b c d\nEXPORT de f\n",
                ObjectsHeader(
                    Version(3, 4),
                    extern=frozenset(("a", "b", "c", "d")),
                    export=frozenset(("de", "f")),
                ),
            ),
            (
                "with export and extern reversed",
                "OBJECTS 3.4\nEXPORT de f\nEXTERN a b c d\n",
                ObjectsHeader(
                    Version(3, 4),
                    extern=frozenset(("a", "b", "c", "d")),
                    export=frozenset(("de", "f")),
                ),
            ),
        ]
    )
    def test_parse_exec_header(self, name: str, source: str, expected: ObjectsHeader):
        parsed = self.parser.parse(self.lexer.tokenize(source)).header
        self.assertEqual(parsed, expected)
