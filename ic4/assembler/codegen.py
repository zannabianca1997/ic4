"""
    Generate intcode from an ast
"""
from distutils.log import warn
from typing import Dict, Iterable, List, Tuple

from .commands import (
    Command,
    Directive,
    DirectiveCode,
    Instruction,
    Label,
    OpCode,
    ParamMode,
)
from .expressions import Expression, SimplifyException, Sum, simplify


class GenerateException(Exception):
    pass


def generate_instruction(instr: Instruction) -> Tuple[Expression]:
    """Generate the code for an instruction"""
    opcode = int(instr.opcode)
    for i, (mode, _) in enumerate(instr.params):
        opcode += 100 * 10**i * int(mode)
    return (opcode, *(val for _, val in instr.params))


def generate(commands: Iterable[Command]) -> Tuple[int, ...]:
    """Generate a intcode program from a list of commands"""
    labels: Dict[str, int] = {}  # known labels
    code: List[Expression] = []  # code generated
    for command in commands:
        if isinstance(command, Instruction):
            code.extend(generate_instruction(command))
        if isinstance(command, Label):
            if command.name in labels:
                warn(f"Label '{command.name}' is redefined")
            labels[command.name] = len(code)  # point to the next instruction
        if isinstance(command, Directive):
            if command.code == DirectiveCode.INTS:
                code.extend(command.params)
            elif command.code == DirectiveCode.ZEROS:
                (numzero,) = command.params
                try:
                    numzero = simplify(numzero, labels, full_simplify=True)
                except SimplifyException as e:
                    raise GenerateException(
                        "ZEROS need an expression in term of already seen labels"
                    ) from e
                if numzero < 0:
                    raise GenerateException("ZEROS need a positive expression")
                code.extend([0] * numzero)
            elif command.code == DirectiveCode.INC:
                code.extend(
                    generate_instruction(
                        Instruction(
                            OpCode.ADD,
                            (
                                command.params[0],
                                (ParamMode.IMMEDIATE, 1),
                                command.params[0],
                            ),
                        )
                    )
                )
            elif command.code == DirectiveCode.DEC:
                code.extend(
                    generate_instruction(
                        Instruction(
                            OpCode.ADD,
                            (
                                command.params[0],
                                (ParamMode.IMMEDIATE, -1),
                                command.params[0],
                            ),
                        )
                    )
                )
            elif command.code == DirectiveCode.MOV:
                # extracting mode and position of both params
                m1, p1 = command.params[0]
                m2, p2 = command.params[1]
                # generating add instructions as needed
                for i in range(command.params[2]):
                    code.extend(
                        generate_instruction(
                            Instruction(
                                OpCode.ADD,  # using add to move stuff. MUL a 1 a would be ok too
                                (
                                    (m1, Sum(p1, i)),
                                    (ParamMode.IMMEDIATE, 0),
                                    (m2, Sum(p2, i)),
                                ),
                            )
                        )
                    )
    return tuple(simplify(val, labels, full_simplify=True) for val in code)
