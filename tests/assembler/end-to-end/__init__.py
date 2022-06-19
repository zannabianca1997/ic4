from itertools import count, takewhile
from os import getenv
from warnings import warn
from pathlib import Path
from typing import Iterable, Tuple, Optional, Union
from unittest import TestCase

from parameterized import parameterized
from pydantic import BaseModel, validator, parse_file_as

from ic4.assembler.lexer import Lexer
from ic4.machine import Machine


class IOExample(BaseModel):
    name: str
    descr: Optional[str] = None
    input: Union[Tuple[int, ...], str] = ()
    output: Union[Tuple[int, ...], str]

    @validator('input')
    def input_is_tuple(cls, inp: Union[Iterable[int], str]):
        if isinstance(inp, str):
            return tuple(ord(ch) for ch in inp)
        try:
            return tuple(inp)
        except TypeError:
            raise ValueError(
                f"input should be a list of integers or a string, not {inp.__class__}")

    @validator('output')
    def output_is_tuple(cls, out: Union[Iterable[int], str]):
        if isinstance(out, str):
            return tuple(ord(ch) for ch in out)
        try:
            return tuple(out)
        except TypeError:
            raise ValueError(
                f"output should be a list of integers or a string, not {out.__class__}")


def get_sources(dir: Path):
    yield from Path(dir).parent.glob("*.ica")


def get_name_path_and_source(dir: Path):
    for source in get_sources(dir):
        with open(source) as source_file:
            source_code = source_file.read()
        yield source.stem, source, source_code


def get_name_source_and_examples(dir: Path):
    for source in get_sources(dir):
        if not source.with_suffix(".json").exists():
            warn(f"{source!s} misses a companion .json file!")
            continue
        with open(source) as source_file:
            source_code = source_file.read()
        io_examples = parse_file_as(
            Tuple[IOExample, ...], source.with_suffix(".json"))
        yield source.stem, source_code, io_examples


class TestExamplePrograms(TestCase):
    machine: Machine

    @parameterized.expand(
        get_name_path_and_source(__file__)
    )
    def test_lex(self, name: str, path: Path, program: str) -> None:
        lexed = Lexer(debug=True)(program)

        log_file_path = Path(getenv("LOG_DIR"))/"tests" / \
            path.relative_to(
                Path(getenv("TEST_DIR")).absolute()).with_suffix("") / "lexed.txt"
        log_file_path.parent.mkdir(exist_ok=True, parents=True)
        with open(log_file_path, "w") as log_file:
            print(*lexed, file=log_file, sep='\n')
