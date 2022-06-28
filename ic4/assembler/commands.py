"""
    Define basic instruction datatype
"""
from dataclasses import dataclass
from enum import IntEnum, Enum, auto
from typing import Tuple, Optional, Union

from py import code

from .expressions import Expression, simplify


class OpCode(IntEnum):
    ADD = 1
    MUL = 2
    IN = 3
    OUT = 4
    JNZ = 5
    JZ = 6
    SLT = 7
    SEQ = 8
    INCB = 9
    HALT = 99

    def param_number(self) -> int:
        return {
            OpCode.ADD: 3,
            OpCode.MUL: 3,
            OpCode.IN: 1,
            OpCode.OUT: 1,
            OpCode.JZ: 2,
            OpCode.JNZ: 2,
            OpCode.SLT: 3,
            OpCode.SEQ: 3,
            OpCode.INCB: 1,
            OpCode.HALT: 0,
        }[self]


class ParamMode(IntEnum):
    ABSOLUTE = 0
    IMMEDIATE = 1
    RELATIVE = 2

    def prefix(self) -> str:
        return {
            ParamMode.ABSOLUTE: "",
            ParamMode.IMMEDIATE: "#",
            ParamMode.RELATIVE: "@",
        }[self]


INSTRUCTION_PARAMS_WRITING_TABLE = {
    OpCode.ADD: (False, False, True),
    OpCode.MUL: (False, False, True),
    OpCode.IN: (True,),
    OpCode.OUT: (False,),
    OpCode.JZ: (False, False),
    OpCode.JNZ: (False, False),
    OpCode.SLT: (False, False, True),
    OpCode.SEQ: (False, False, True),
    OpCode.INCB: (False,),
    OpCode.HALT: (),
}


@dataclass(frozen=True)
class Instruction:
    opcode: OpCode
    params: Tuple[Tuple[ParamMode, Expression], ...]

    def params_check(self, check_ranges=True, **_) -> None:
        """Checks if all the params are valid (e.g. no immediate on writing parameters or negative on absolute)"""
        assert (
            len(self.params) == self.opcode.param_number()
        ), f"Instruction {self.opcode.name} expected {self.opcode.param_number()} params, got {len(self.params)}"

        # checks if a param is used for writing
        for (mode, _), writing in zip(
            self.params, INSTRUCTION_PARAMS_WRITING_TABLE[self.opcode]
        ):
            if writing:
                assert (
                    mode != ParamMode.IMMEDIATE
                ), f"Instruction {self.opcode.name} do not accept IMMEDIATE mode on output parameter"

        if check_ranges:
            for mode, value in self.params:
                if mode == ParamMode.ABSOLUTE:
                    value = simplify(value)
                    if isinstance(value, int):
                        assert value >= 0, "Negative value on absolute parameter"


class DirectiveCode(Enum):
    INTS = auto()
    ZEROS = auto()
    INC = auto()
    DEC = auto()
    MOV = auto()
    LOAD = auto()
    STORE = auto()
    JMP = auto()
    PUSH = auto()
    POP = auto()


@dataclass(frozen=True)
class Directive:
    code: DirectiveCode
    params: Tuple[Union[Tuple[ParamMode, Expression], Expression], ...]

    def params_check(self, **_) -> None:
        """Check that the parameters are of the right type"""
        if self.code == DirectiveCode.ZEROS:
            assert len(self.params) == 1
            p = simplify(self.params[0])
            if isinstance(p, int):
                assert p >= 0
        if self.code in {DirectiveCode.DEC, DirectiveCode.INC, DirectiveCode.JMP}:
            assert len(self.params) == 1
            (param,) = self.params
            assert isinstance(param, tuple) and len(param) == 2
            mode, _ = param
            assert isinstance(mode, ParamMode)
        if self.code == DirectiveCode.MOV:
            assert len(self.params) == 3
            p1, p2, p3 = self.params
            for p in p1, p2:
                assert isinstance(p, tuple) and len(p) == 2
                mode, _ = p
                assert isinstance(mode, ParamMode)
            assert p2[0] != ParamMode.IMMEDIATE
            p3 = simplify(p3)
            if isinstance(p3, int):
                assert p3 >= 0
        if self.code in {DirectiveCode.LOAD, DirectiveCode.STORE}:
            assert len(self.params) == 2
            src, dest = self.params
            for p in src, dest:
                assert isinstance(p, tuple) and len(p) == 2
                mode, _ = p
                assert isinstance(mode, ParamMode)
            if self.code == DirectiveCode.STORE:
                assert dest[0] != ParamMode.IMMEDIATE
        if self.code in {DirectiveCode.PUSH, DirectiveCode.POP}:
            assert len(self.params) == 2
            data, leng = self.params
            if data is not None:
                assert isinstance(data, tuple) and len(data) == 2
                mode, _ = data
                assert isinstance(mode, ParamMode)
                if code == DirectiveCode.POP:
                    assert mode != ParamMode.IMMEDIATE
            leng = simplify(leng)
            if isinstance(leng, int):
                assert leng >= 0


@dataclass(frozen=True)
class Label:
    name: str

    def params_check(self, **_) -> None:
        pass


Command = Union[Label, Directive, Instruction]
