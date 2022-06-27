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


def generate_instruction(instr: Instruction) -> Tuple[Expression, ...]:
    """Generate the code for an instruction"""
    opcode = int(instr.opcode)
    for i, (mode, _) in enumerate(instr.params):
        opcode += 100 * 10**i * int(mode)
    return (opcode, *(val for _, val in instr.params))


def generate_directive(
    directive: Directive, labels: Dict[str, int] = None
) -> Tuple[Expression, ...]:
    if directive.code == DirectiveCode.INTS:
        return directive.params
    elif directive.code == DirectiveCode.ZEROS:
        (numzero,) = directive.params
        try:
            numzero = simplify(numzero, labels, full_simplify=True)
        except SimplifyException as e:
            raise GenerateException(
                "ZEROS need an expression in term of already seen labels"
            ) from e
        if numzero < 0:
            raise GenerateException("ZEROS need a positive expression")
        return tuple([0] * numzero)
    elif directive.code == DirectiveCode.INC:
        return generate_instruction(
            Instruction(
                OpCode.ADD,
                (
                    directive.params[0],
                    (ParamMode.IMMEDIATE, 1),
                    directive.params[0],
                ),
            )
        )
    elif directive.code == DirectiveCode.DEC:
        return generate_instruction(
            Instruction(
                OpCode.ADD,
                (
                    directive.params[0],
                    (ParamMode.IMMEDIATE, -1),
                    directive.params[0],
                ),
            )
        )
    elif directive.code == DirectiveCode.MOV:
        # extracting mode and position of both params
        m1, p1 = directive.params[0]
        m2, p2 = directive.params[1]
        # storing code generated
        code = []
        # generating add instructions as needed
        for i in range(directive.params[2]):
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
        return tuple(code)
    else:
        raise NotImplementedError(f"Directive {directive.code} it's unimplemented")


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
            code.extend(generate_directive(command, labels))
    return tuple(simplify(val, labels, full_simplify=True) for val in code)
