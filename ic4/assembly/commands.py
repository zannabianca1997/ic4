"""
    Define basic instruction datatype
"""
from dataclasses import dataclass
from enum import IntEnum, unique
from string import printable
from typing import Optional, Tuple

from numpy import size

from ic4.string_utilities import escape_string_const

from .expressions import Constant, Expression, Reference, SimplifyException


@unique
class OpCode(IntEnum):
    """The code of an instruction"""

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

    @property
    def param_number(self) -> int:
        """Get the number of params this opcode require

        :return: number of params
        :rtype: int
        """
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

    @property
    def writes_to(self) -> Tuple[bool, ...]:
        """Return a tuple indicating what params are used for writing (True for writing needed).

        :return: the tuple
        :rtype: Tuple[bool, ...]
        """
        return {
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
        }[self]

    def __str__(self) -> str:
        return self.name


@unique
class ParamMode(IntEnum):
    """Mode of a parameter"""

    ABSOLUTE = 0
    IMMEDIATE = 1
    RELATIVE = 2

    @property
    def prefix(self) -> str:
        """Prefix used in assembly language for this mode

        :return: the prefix
        :rtype: str
        """
        return {
            ParamMode.ABSOLUTE: "",
            ParamMode.IMMEDIATE: "#",
            ParamMode.RELATIVE: "@",
        }[self]

    def __str__(self) -> str:
        return self.prefix

    @property
    def can_be_written(self) -> bool:
        return self is not ParamMode.IMMEDIATE


@dataclass(frozen=True)
class Param:
    mode: ParamMode
    value: Expression

    @property
    def can_be_written(self) -> bool:
        return self.mode.can_be_written

    def check(self) -> None:
        """Perform some basic checks on this parameter

        :raises: AssertionError if the check failed
        """
        if self.mode is ParamMode.ABSOLUTE:
            # value cannot be negative
            try:
                val = self.value.simplify(fullsimplify=True)
            except SimplifyException:
                pass  # do not panic, it's simply a complex parameter
            else:
                assert val.value >= 0, "Absolute parameters cannot have negative values"

    def __str__(self) -> str:
        return f"{self.mode}{self.value}"


@dataclass(frozen=True)
class Command:
    """Represent a command in a file"""

    def check(self) -> None:
        """Perform some basic checks on this command

        :raises: AssertionError if the check failed
        """
        raise NotImplementedError()


@dataclass(frozen=True)
class Label(Command):
    """Represent a label"""

    value: str

    def check(self) -> None:
        return Reference.INTERNAL_NAMES_RE.fullmatch(self.value)

    def __str__(self) -> str:
        return f"{self.value}: "


@dataclass(frozen=True)
class Instruction(Command):
    """Represent a instruction"""

    opcode: OpCode
    params: Tuple[Param, ...]

    def check(self) -> None:
        # check number of params
        assert len(self.params) == self.opcode.param_number
        # checks all params that are used for writing can be written
        assert all(
            (not writes_to) or p.can_be_written
            for writes_to, p in zip(self.opcode.writes_to, self.params)
        )
        # self check of the params
        for p in self.params:
            p.check()

    def __str__(self) -> str:
        return str(self.opcode) + " " + ", ".join(str(p) for p in self.params) + "\n"


@dataclass(frozen=True)
class Directive(Command):
    """Represent an assambler directive"""

    def __str__(self) -> str:
        return f"{self.__class__.__name__} {self._param_str()}\n"

    def _param_str(self) -> str:
        raise NotImplementedError()


@dataclass(frozen=True)
class INTS(Directive):
    """Insert the given numbers verbatim"""

    values: Tuple[Expression, ...]

    def check(self) -> None:
        pass

    def _param_str(self) -> str:
        if all(
            (isinstance(x, Constant) and (chr(x.value) in (printable + "\0")))
            for x in self.values
        ):
            return escape_string_const(x.value for x in self.values)
        return ", ".join(str(v) for v in self.values)


@dataclass(frozen=True)
class ZEROS(Directive):
    """Insert a given number of zero"""

    len: Expression

    def check(self) -> None:
        # len cannot be negative
        try:
            val = self.len.simplify(fullsimplify=True)
        except SimplifyException:
            pass  # do not panic, it's simply a complex directive
        else:
            assert val.value >= 0, "Cannot insert a negative number of zeros"

    def _param_str(self) -> str:
        return str(self.len)


@dataclass(frozen=True)
class JMP(Directive):
    """Unconditional jump"""

    dest: Param

    def check(self) -> None:
        self.dest.check()

    def _param_str(self) -> str:
        return str(self.dest)


@dataclass(frozen=True)
class INC(Directive):
    """Increment"""

    dest: Param

    def check(self) -> None:
        self.dest.check()
        assert self.dest.can_be_written

    def _param_str(self) -> str:
        return str(self.dest)


@dataclass(frozen=True)
class DEC(Directive):
    """Decrement"""

    dest: Param

    def check(self) -> None:
        self.dest.check()
        assert self.dest.can_be_written

    def _param_str(self) -> str:
        return str(self.dest)


@dataclass(frozen=True)
class MOV(Directive):
    """Memory move"""

    src: Param
    dest: Param
    size: Expression = Constant(1)

    def check(self) -> None:
        for p in self.src, self.dest:
            p.check()
        assert self.dest.can_be_written
        # size cannot be negative
        try:
            val = self.size.simplify(fullsimplify=True)
        except SimplifyException:
            pass  # do not panic, it's simply a complex directive
        else:
            assert val.value >= 0, "Cannot move a negative number of memory locations"

    def _param_str(self) -> str:
        return f"{self.src}, {self.dest}" + (f", {self.size}" if self.size != 1 else "")


@dataclass(frozen=True)
class LOAD(Directive):
    """Load from a pointer cell"""

    src_ptr: Param
    dest: Param

    def check(self) -> None:
        for p in self.src_ptr, self.dest:
            p.check()
        assert self.dest.can_be_written

    def _param_str(self) -> str:
        return f"{self.src_ptr}, {self.dest}"


@dataclass(frozen=True)
class STORE(Directive):
    """Store to a pointer cell"""

    src: Param
    dest_ptr: Param

    def check(self) -> None:
        for p in self.src, self.dest_ptr:
            p.check()

    def _param_str(self) -> str:
        return f"{self.src}, {self.dest_ptr}"


@dataclass(frozen=True)
class PUSH(Directive):
    """Push a value onto the stack"""

    value: Optional[Param] = None
    size: Expression = Constant(1)

    def check(self) -> None:
        if self.value is not None:
            self.value.check()
        # size cannot be negative
        try:
            val = self.size.simplify(fullsimplify=True)
        except SimplifyException:
            pass  # do not panic, it's simply a complex directive
        else:
            assert val.value >= 0, "Cannot push a negative number of memory locations"

    def _param_str(self) -> str:
        return str(self.value) + (f", {self.size}" if self.size != 1 else "")


@dataclass(frozen=True)
class POP(Directive):
    """Pop a value from the stack"""

    dest: Optional[Param] = None
    size: Expression = Constant(1)

    def check(self) -> None:
        if self.dest is not None:
            self.dest.check()
            assert self.dest.can_be_written
        # size cannot be negative
        try:
            val = self.size.simplify(fullsimplify=True)
        except SimplifyException:
            pass  # do not panic, it's simply a complex directive
        else:
            assert val.value >= 0, "Cannot pop a negative number of memory locations"

    def _param_str(self) -> str:
        return str(self.dest) + (f", {self.size}" if self.size != 1 else "")


@dataclass(frozen=True)
class CALL(Directive):
    """Call a subroutine"""

    dest: Param

    def check(self) -> None:
        self.dest.check()

    def _param_str(self) -> str:
        return str(self.dest)


@dataclass(frozen=True)
class RET(Directive):
    """Return from a subroutine"""

    def check(self) -> None:
        pass

    def _param_str(self) -> str:
        return ""
