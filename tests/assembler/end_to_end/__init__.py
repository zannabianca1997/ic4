from itertools import count, takewhile
from os import getenv
from warnings import warn
from pathlib import Path
from typing import Iterable, Tuple, Optional, Union
from unittest import TestCase

from parameterized import parameterized
from pydantic import BaseModel, validator, parse_file_as
from sly.lex import Token
from ic4.assembler.codegen import generate
from ic4.assembler.commands import Directive, Instruction, Label

from ic4.assembler.lexer import ICAssLexer
from ic4.assembler.parser import ICAssParser
from ic4.machine import Machine


class IOExample(BaseModel):
    name: str
    descr: Optional[str] = None
    input: Union[Tuple[int, ...], str] = ()
    output: Union[Tuple[int, ...], str]

    @validator("input")
    def input_is_tuple(cls, inp: Union[Iterable[int], str]):
        if isinstance(inp, str):
            return tuple(ord(ch) for ch in inp)
        try:
            return tuple(inp)
        except TypeError:
            raise ValueError(
                f"input should be a list of integers or a string, not {inp.__class__}"
            )

    @validator("output")
    def output_is_tuple(cls, out: Union[Iterable[int], str]):
        if isinstance(out, str):
            return tuple(ord(ch) for ch in out)
        try:
            return tuple(out)
        except TypeError:
            raise ValueError(
                f"output should be a list of integers or a string, not {out.__class__}"
            )


def get_sources(dir: Path):
    yield from Path(dir).parent.glob("*.ica")


def get_log_path(file_path: Path) -> Path:
    log_path = (
        Path(getenv("LOG_DIR"))
        / "tests"
        / file_path.relative_to(Path(getenv("TEST_DIR")).absolute()).with_suffix("")
    )
    log_path.mkdir(exist_ok=True, parents=True)
    return log_path


def get_name_log_path_and_source(dir: Path):
    for source in get_sources(dir):
        with open(source) as source_file:
            source_code = source_file.read()
        yield source.stem, get_log_path(source), source_code


def get_name_source_and_example(dir: Path):
    for source in get_sources(dir):
        if not source.with_suffix(".json").exists():
            warn(f"{source!s} misses a companion .json file!")
            continue
        with open(source) as source_file:
            source_code = source_file.read()
        for io_example in parse_file_as(
            Tuple[IOExample, ...], source.with_suffix(".json")
        ):
            yield source.stem + "_" + io_example.name, source_code, io_example


class TestLexParse(TestCase):
    @parameterized.expand(get_name_log_path_and_source(__file__))
    def test_lex(self, name: str, log_path: Path, program: str) -> None:
        """Main purpose of this test is that the program must be lexed without errors.
        As a side effect, the lexed program is logged"""
        with open(log_path / "lexed.txt", "w") as log_file:
            for tok in ICAssLexer().tokenize(program):
                self.assertIsInstance(tok, Token)
                print(tok, file=log_file)

    @parameterized.expand(get_name_log_path_and_source(__file__))
    def test_parse(self, name: str, log_path: Path, program: str) -> None:
        """Main purpose of this test is that the program must be parsed without errors.
        As a side effect, the parsed program is logged"""
        with open(log_path / "parsed.txt", "w") as log_file:
            for command in ICAssParser().parse(ICAssLexer().tokenize(program)):
                self.assert_(
                    any(
                        isinstance(command, cls)
                        for cls in (Label, Instruction, Directive)
                    )
                )
                print(command, file=log_file)

    @parameterized.expand(get_name_log_path_and_source(__file__))
    def test_compile(self, name: str, log_path: Path, program: str) -> None:
        """Main purpose of this test is that the program must be compiled without errors.
        As a side effect, the compiled program is logged"""
        compiled = generate(ICAssParser().parse(ICAssLexer().tokenize(program)))
        with open(log_path / "compiled.txt", "w") as log_file:
            print(*compiled, sep=", ", end="", file=log_file)
        for x in compiled:
            self.assertIsInstance(x, int)


class TestRun(TestCase):
    @parameterized.expand(get_name_source_and_example(__file__))
    def test_run(self, name: str, program: str, io_example: IOExample):
        """Compile the program and test it against the given example input/output"""
        machine = Machine(generate(ICAssParser().parse(ICAssLexer().tokenize(program))))
        machine.give_input(io_example.input)
        machine.run()
        self.assert_(machine.halted, "The machine did not halt with the given input!")
        self.assertTupleEqual(
            io_example.output,
            tuple(
                takewhile(
                    lambda x: x is not None, (machine.get_output() for _ in count())
                )
            ),
        )
