import json
from warnings import warn
from itertools import count, takewhile
from pathlib import Path
from typing import Dict, Iterable, Tuple, Optional, Union
from unittest import TestCase, TestSuite, main
from slugify import slugify
from pydantic import BaseModel, validator

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
        if isinstance(inp, Iterable[int]):
            return tuple(inp)
        raise ValueError(
            f"input should be a list of integers or a string, not {inp.__class__}")

    @validator('output')
    def output_is_tuple(cls, out: Union[Iterable[int], str]):
        if isinstance(out, str):
            return tuple(ord(ch) for ch in out)
        if isinstance(out, Iterable[int]):
            return tuple(out)
        raise ValueError(
            f"output should be a list of integers or a string, not {out.__class__}")


def _make_test_case(program_name: str, program: Tuple[int, ...], ioexamples: Iterable[IOExample]):
    """Create a test case"""
    class TestProgram(TestCase):
        _machine: Machine
        _program: Tuple[int] = program
        _ioexamples: Dict[str, IOExample] = {
            example.name: example for example in ioexamples}

        def setUp(self) -> None:
            # initialize the intcode machine
            self._machine = Machine(self._program)

    test_funcs = []
    for name, example in TestProgram._ioexamples.items():
        def test_fun(self: TestProgram):
            self._machine.give_input(example.input)
            output = tuple(takewhile(lambda x: x is not None, (self._machine.get_output()
                                                               for _ in count())))
            self.assertTupleEqual(
                output, self._ioexamples[name].output, f"Test:{self._ioexamples[name].name}\n{self._ioexamples[name].descr or ''}\nOutput is different\n")
        fname = "test_" + slugify(
            example.name, separator="_",
            regex_pattern=r"[^a-zA-Z0-9_]+"
        )
        # resolve duplicates
        other_names = set(fun.__name__ for fun in test_funcs)
        if fname in other_names:
            i = 1
            while f"{fname}_{i}" in other_names:
                i += 1
            fname = f"{fname}_{i}"
        test_fun.__name__ = fname
        test_funcs.append(test_fun)
    # adding functions
    for fun in test_funcs:
        setattr(TestProgram, fun.__name__, fun)
    # setting name
    TestProgram.__name__ = "Test" + slugify(
        program_name.capitalize().replace(" ", ""), separator="_",
        regex_pattern=r"[^a-zA-Z0-9_]+",
        lowercase=False
    )
    return TestProgram


# recovering all cases
test_cases = []
for source in Path(__file__).parent.glob("*.int"):
    if not source.with_suffix(".json").exists():
        warn(f"{source} does not have a connected json file")
        continue

    with open(source.with_suffix(".json")) as examplefile:
        _ioexamples = tuple(IOExample.parse_obj(x)
                            for x in json.load(examplefile))
    with open(source) as sourcefile:
        _program = tuple(int(x.strip())
                         for x in sourcefile.read().split(",") if x.strip())

    test_cases.append(_make_test_case(
        source.stem,
        _program,
        _ioexamples
    ))

del _make_test_case, _ioexamples, _program, source, examplefile, sourcefile


def load_tests(loader, tests, pattern):
    suite = TestSuite()
    for test_class in test_cases:
        tests = loader.loadTestsFromTestCase(test_class)
        suite.addTests(tests)
    return suite
