"""
    Define basic instruction datatype
"""
from dataclasses import dataclass
from enum import IntEnum
from typing import Tuple

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
        return {OpCode.ADD: 3,
                OpCode.MUL: 3,
                OpCode.IN: 1,
                OpCode.OUT: 1,
                OpCode.JZ: 2,
                OpCode.JNZ: 2,
                OpCode.SLT: 3,
                OpCode.SEQ: 3,
                OpCode.INCB: 1,
                OpCode.HALT: 0
                }[self]


class ParamMode(IntEnum):
    MODE0 = 0
    MODE1 = 1
    MODE2 = 2

    def prefix(self) -> str:
        return {
            ParamMode.MODE0: "",
            ParamMode.MODE1: "#",
            ParamMode.MODE2: "@"
        }[self]


@dataclass(frozen=True)
class Instruction:
    opcode: OpCode
    params: Tuple[ParamMode, Expression]
