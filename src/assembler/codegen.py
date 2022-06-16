"""
    Generate intcode from an ast
"""
from distutils.log import warn
from typing import Dict, Iterable, List, Tuple

from .commands import Command, Directive, DirectiveCode, Instruction, Label
from .expressions import Expression, simplify


def generate_instruction(instr: Instruction) -> Tuple[Expression]:
    """Generate the code for an instruction"""
    opcode = int(instr.opcode)
    for i, (mode, _) in enumerate(instr.params):
        opcode += 100 * 10**i * int(mode)
    return (
        opcode,
        *(val for _, val in instr.params)
    )


def generate(commands: Iterable[Command]) -> Tuple[int]:
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
    return tuple(
        simplify(val, labels, full_simplify=True)
        for val in code
    )
