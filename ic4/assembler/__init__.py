from typing import Tuple

from .lexer import ICAssLexer
from .parser import ICAssParser
from .codegen import generate


def compile(source: str) -> Tuple[int, ...]:
    return generate(ICAssParser().parse(ICAssLexer().tokenize(source)))
