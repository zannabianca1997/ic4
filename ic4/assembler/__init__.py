
from typing import Tuple

from .codegen import generate
from .parser import parse


def assemble(code: str) -> Tuple[int, ...]:
    return generate(parse(code))
