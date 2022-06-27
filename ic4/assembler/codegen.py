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
from .expressions import Expression, SimplifyException, Sum, simplify, simplify_tuple


class GenerateException(Exception):
    pass


def generate_instruction(instr: Instruction) -> Tuple[Expression, ...]:
    """Generate the code for an instruction"""
    opcode = int(instr.opcode)
    for i, (mode, _) in enumerate(instr.params):
        opcode += 100 * 10**i * int(mode)
    return (opcode, *(val for _, val in instr.params))


def generate_directive(
    directive: Directive, labels: Dict[str, int], pos: int
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
        for i in range(simplify(directive.params[2], labels, full_simplify=True)):
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
    elif directive.code == DirectiveCode.LOAD:
        code = []
        code.extend(
            generate_directive(  # code to move the address into the code
                Directive(
                    DirectiveCode.MOV,
                    (
                        directive.params[0],
                        (
                            ParamMode.ABSOLUTE,
                            "address_pos",  # placeholder for the position of the destination of the address
                            # MOV do not expand any param, except the third
                        ),
                        1,
                    ),
                ),
                {},
                pos,
            )
        )
        code.extend(
            generate_directive(
                Directive(
                    DirectiveCode.MOV,
                    (
                        (
                            ParamMode.ABSOLUTE,
                            "address_dest",  # placeholder for where the code will put the address.
                            # MOV do not expand any param, except the third
                        ),
                        directive.params[1],
                        1,
                    ),
                ),
                labels,
                pos,
            )
        )
        code = list(simplify_tuple(code))  # this will erase the +0 MOV tend to add
        # patching code
        address_dest = code.index("address_dest")
        code[code.index("address_pos")] = pos + address_dest
        code[address_dest] = -1  # this will be substituted by the code.
        # If not, it will create a invalid reference (the first time around)
        # here we can generate code to put the -1 back. Should we? I think not, or at least it should be higly optional
        return code
    else:
        raise NotImplementedError(f"Directive {directive.code} it's unimplemented")


def generate(
    commands: Iterable[Command], full_simplify: bool = True
) -> Tuple[Expression, ...]:
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
            code.extend(generate_directive(command, labels, len(code)))
    return simplify_tuple(code, labels, full_simplify=full_simplify)
