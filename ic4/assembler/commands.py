"""
    Define basic instruction datatype
"""
from dataclasses import dataclass
from enum import IntEnum, Enum, auto
from typing import Tuple, Optional, Union

from .expressions import Expression


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


@dataclass(frozen=True)
class Instruction:
    opcode: OpCode
    params: Tuple[Tuple[ParamMode, Expression], ...]


class DirectiveCode(Enum):
    INTS = auto()
    ZEROS = auto()
    INC = auto()
    DEC = auto()
    MOV = auto()
    LOAD = auto()
    STORE = auto()
    JMP = auto()


@dataclass(frozen=True)
class Directive:
    code: DirectiveCode
    params: Tuple[Union[Tuple[ParamMode, Expression], Expression], ...]


@dataclass(frozen=True)
class Label:
    name: str


Command = Union[Label, Directive, Instruction]
