from itertools import count, takewhile
from warnings import warn
from pathlib import Path
from typing import Iterable, Tuple, Optional, Union
from unittest import TestCase

from parameterized import parameterized
from pydantic import BaseModel, validator, parse_file_as
from ic4.assembler import assemble

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


def get_source_and_examples(dir: Path):
    for source in Path(dir).parent.glob("*.ica"):
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
        get_source_and_examples(__file__)
    )
    def test_output(self, name: str, program: str, ioexamples: Iterable[IOExample]) -> None:
        program = assemble(program)
        for ioexample in ioexamples:
            with self.subTest(ioexample.name):
                machine = Machine(program)
                machine.give_input(ioexample.input)
                output = tuple(takewhile(lambda x: x is not None,
                               (machine.get_output() for _ in count())))
                self.assertTupleEqual(
                    output, ioexample.output
                )
