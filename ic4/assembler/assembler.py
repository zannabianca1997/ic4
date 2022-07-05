from itertools import count
from typing import Dict, Iterator, List, Optional, Tuple

from ..version import Version

from ..assembly.expressions import Constant, Expression, Reference, SimplifyException
from ..assembly.commands import (
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
    Command,
    Directive,
    Instruction,
    Label,
    OpCode,
    Param,
    ParamMode,
)
from ..assembly.srcfile import SourceFile


class GenerateException(Exception):
    pass


class Assembler:
    _command_stack: List[Command]  # commands to go through
    _values: Optional[Tuple[int, ...]]  # values ready to be emitted
    _labels: Dict[Reference, Expression]  # labels meeted
    _names: Iterator[str]  # unique names to be used with internal labels

    def __init__(self, source_file: SourceFile) -> None:
        if not source_file.is_executable:
            raise TypeError("Source file must be an executable to be assembled")
        if source_file.header.version != Version(0, 1):
            raise NotImplementedError(
                "Assembler is only implemented for assembly version 0.1"
            )
        self._pos = 0
        self._command_stack = list(reversed(source_file.body))
        self._values = None
        self._labels = {}
        self._names = (f"&{i}" for i in count())

    def __iter__(self) -> Iterator[int]:
        yield from self.values

    @property
    def values(self) -> Tuple[int, ...]:
        if self._values is None:
            self.generate()
        return self._values

    def generate(self) -> Tuple[int, ...]:
        """Assemble the program"""
        code: List[Expression] = []

        # generate the code, left as expression
        while self._command_stack:  # until there are other commands
            comm = self._command_stack.pop()
            if isinstance(comm, Instruction):
                code.extend(self.gen_instruction(comm))
            elif isinstance(comm, Label):
                self._labels[Reference(comm.value)] = Constant(len(code))
            elif isinstance(comm, Directive):
                method_name = f"gen_{comm.__class__.__name__}"
                try:
                    method = getattr(self, method_name)
                except AttributeError:
                    raise NotImplementedError(
                        f"Generation for directive {comm.__class__.__name__} is missing"
                    )
                code.extend(method(comm))
            else:
                assert False, "Unknow command type"

        """
        logfile = open("logs/assembler_log.txt", "w")
        print(*(str(x) for x in code), file=logfile, sep="\n")
        logfile.close()
        """

        # substitute the expressions
        try:
            self._values = tuple(expr.simplify(self._labels).value for expr in code)
        except SimplifyException as e:
            raise GenerateException(
                f"Cannot simplify the final code to ints, is a label missing?"
            ) from e

        return self.values

    def gen_instruction(self, instr: Instruction) -> Tuple[Expression, ...]:
        """Generate the code for a single instruction

        :param instr: The instruction to generate
        :type instr: Instruction
        :return: The generated code
        :rtype: Tuple[int, ...]
        """
        # adding param modes to opcode
        op = (
            100
            * (
                (
                    10
                    * (
                        (
                            10
                            * (
                                instr.params[2].mode.value
                                if len(instr.params) > 2
                                else 0
                            )
                            + instr.params[1].mode.value
                        )
                        if len(instr.params) > 1
                        else 0
                    )
                    + instr.params[0].mode.value
                )
                if len(instr.params) > 0
                else 0
            )
            + instr.opcode.value
        )
        return (Constant(op), *(p.value for p in instr.params))

    def gen_INTS(self, direct: INTS) -> Tuple[Expression, ...]:
        return direct.values  # copy the values verbatim

    def gen_ZEROS(self, direct: ZEROS) -> Tuple[Expression, ...]:
        try:
            num_zeros = direct.len.simplify(self._labels).value
        except SimplifyException as e:
            raise GenerateException(
                f"ZEROS parameter need to be simplifiable to an int"
            ) from e
        if num_zeros < 0:
            raise GenerateException(f"ZEROS parameter need to be positive")
        return tuple([Constant(0)] * num_zeros)

    def gen_INC(self, direct: INC) -> Tuple[()]:
        self._command_stack.append(
            Instruction(
                OpCode.ADD,
                (direct.dest, Param(ParamMode.IMMEDIATE, Constant(1)), direct.dest),
            )
        )
        return ()  # no code to add directly

    def gen_DEC(self, direct: DEC) -> Tuple[()]:
        self._command_stack.append(
            Instruction(
                OpCode.ADD,
                (direct.dest, Param(ParamMode.IMMEDIATE, Constant(-1)), direct.dest),
            )
        )
        return ()  # no code to add directly

    def gen_MOV(self, direct: MOV) -> Tuple[()]:
        try:
            size = direct.size.simplify(self._labels).value
        except SimplifyException as e:
            raise GenerateException(
                f"MOV size parameter need to be simplifiable to an int"
            ) from e
        if size < 0:
            raise GenerateException(f"MOV size parameter need to be positive")
        self._command_stack.extend(
            reversed(  # being a stack, they must be added in reverse order.
                [
                    Instruction(
                        OpCode.ADD,
                        (
                            Param(direct.src.mode, direct.src.value + offset)
                            if direct.src.mode
                            is not ParamMode.IMMEDIATE  # for IMMEDIATE mode the param must remain the same
                            else direct.src,
                            Param(ParamMode.IMMEDIATE, Constant(0)),
                            Param(direct.dest.mode, direct.dest.value + offset),
                        ),
                    )
                    for offset in range(size)
                ]
            )
        )
        return ()  # no code to add directly

    def gen_JMP(self, direct: JMP) -> Tuple[()]:
        self._command_stack.append(
            Instruction(
                OpCode.JNZ,
                (
                    Param(ParamMode.IMMEDIATE, Constant(1)),
                    direct.dest,
                ),
            ),
        )
        return ()  # no code to add directly

    def gen_LOAD(self, direct: LOAD) -> Tuple[()]:
        # LOAD is implemented with self-modifing code
        # Two instruction are generated: one copy the value into the first param of the second
        # the second make the effective move.
        dest_lbl = next(self._names)
        self._command_stack.extend(
            reversed(  # being a stack, they must be added in reverse order.
                [
                    MOV(
                        direct.src_ptr,
                        Param(ParamMode.ABSOLUTE, Reference(dest_lbl) + 1),
                    ),
                    Label(dest_lbl),
                    MOV(
                        Param(
                            ParamMode.ABSOLUTE,
                            Constant(
                                -1  # the -1 will be replaced when the program is run
                                # it's negative so if not working correctly an exception is thrown
                            ),
                        ),
                        direct.dest,
                    ),
                ]
            )
        )
        return ()  # no code to add directly

    def gen_STORE(self, direct: STORE) -> Tuple[()]:
        # STORE is implemented with self-modifing code
        # Two instruction are generated: one copy the value into the second param of the second
        # the second make the effective move.
        dest_lbl = next(self._names)
        self._command_stack.extend(
            reversed(  # being a stack, they must be added in reverse order.
                [
                    MOV(
                        direct.dest_ptr,
                        Param(ParamMode.ABSOLUTE, Reference(dest_lbl) + 3),
                    ),
                    Label(dest_lbl),
                    MOV(
                        direct.src,
                        Param(
                            ParamMode.ABSOLUTE,
                            Constant(
                                -1  # the -1 will be replaced when the program is run
                                # it's negative so if not working correctly an exception is thrown
                            ),
                        ),
                    ),
                ]
            )
        )
        return ()  # no code to add directly

    def gen_PUSH(self, direct: PUSH) -> Tuple[()]:
        # being a stack, they must be added in reverse order.
        self._command_stack.append(
            Instruction(OpCode.INCB, (Param(ParamMode.IMMEDIATE, direct.size),))
        )
        if direct.value is not None:
            # if source is given, then we produce the MOVing directive
            self._command_stack.append(
                MOV(direct.value, Param(ParamMode.RELATIVE, Constant(0)), direct.size)
            )
        return ()  # no code to add directly

    def gen_POP(self, direct: POP) -> Tuple[()]:
        # being a stack, they must be added in reverse order.
        if direct.dest is not None:
            # if source is given, then we produce the MOVing directive
            self._command_stack.append(
                MOV(Param(ParamMode.RELATIVE, Constant(0)), direct.dest, direct.size)
            )
        self._command_stack.append(
            Instruction(OpCode.INCB, (Param(ParamMode.IMMEDIATE, -direct.size),))
        )
        return ()  # no code to add directly

    def gen_CALL(self, direct: CALL) -> Tuple[()]:
        ret_label_name = next(self._names)
        self._command_stack.extend(
            reversed(  # being a stack, they must be added in reverse order.
                [
                    PUSH(Param(ParamMode.IMMEDIATE, Reference(ret_label_name))),
                    JMP(direct.dest),
                    Label(ret_label_name),
                ]
            )
        )
        return ()  # no code to add directly

    def gen_RET(self, direct: RET) -> Tuple[()]:
        self._command_stack.extend(
            reversed(  # being a stack, they must be added in reverse order.
                [
                    POP(),
                    JMP(Param(ParamMode.RELATIVE, Constant(0))),
                ]
            )
        )
        return ()  # no code to add directly
