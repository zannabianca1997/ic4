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
    JZ = 5
    JNZ = 6
    SLT = 7
    SEQ = 8
    INCB = 9
    HALT = 99


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
