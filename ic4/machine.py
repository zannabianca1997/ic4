"""
    Implement a IntCode machine
"""

from itertools import count, takewhile
from os import stat
from typing import TextIO, Union, Iterable, Optional, List


class Machine:
    """A IntCode machine"""

    _memory: List[int]
    _PC: int
    _RB: int
    _halted: bool

    _input: List[int]
    _output: List[int]

    def __init__(self, program: Iterable[int], debug: Optional[TextIO] = None):
        self._memory = list(program)
        self._PC = 0
        self._RB = 0
        self._halted = False

        self._input = []
        self._output = []

        self._debug = debug

    def state_dump(self) -> str:
        """Get a pretty print of the machine state"""
        state = ""
        # log where in the line the pointers are
        PC_start = 0
        RB_start = 0
        # print memory
        for i, v in enumerate(self._memory):
            if i == self._PC:
                PC_start = len(state)
            if i == self._RB:
                RB_start = len(state) + 1
            state += str(v) + ", "
        state += "\n"
        # print pointers
        if PC_start < RB_start:
            state += " " * PC_start + "^" + " " * (RB_start - PC_start - 1) + "@"
        else:
            state += " " * RB_start + "@" + " " * (PC_start - RB_start - 1) + "^"
        state += "\n"

        return state

    def _get_mem(self, idx: int) -> int:
        """Get memory at index"""
        if idx < 0:
            raise IndexError(f"Accessed negative memory location {idx} for reading")
        if len(self._memory) <= idx:  # memory is shorter than what we want
            return 0
        else:
            return self._memory[idx]

    def _set_mem(self, idx: int, val: int) -> None:
        """Set memory at a index"""
        if idx < 0:
            raise IndexError(f"Accessed negative memory location {idx} for writing")
        if len(self._memory) <= idx:  # memory is shorter than what we need
            if val != 0:
                self._memory += [0] * (idx + 1 - len(self._memory))  # pad the list
                self._memory[idx] = val
        else:
            self._memory[idx] = val

    def _get_param(self, idx: int, mode: int) -> int:
        if mode == 0:
            return self._get_mem(self._get_mem(idx))
        if mode == 1:
            return self._get_mem(idx)
        if mode == 2:
            return self._get_mem(self._RB + self._get_mem(idx))
        raise ValueError(f"Invalid param mode {mode}")

    def _set_param(self, idx: int, mode: int, val: int) -> None:
        if mode == 0:
            self._set_mem(self._get_mem(idx), val)
        elif mode == 1:
            raise ValueError(f"Param mode 1 cannot be used for writing!")
        elif mode == 2:
            self._set_mem(self._RB + self._get_mem(idx), val)
        else:
            raise ValueError(f"Invalid param mode {mode}")

    def run(self) -> bool:
        """Run the machine until halted or input is needed. return True if not halted"""
        if self._halted:
            return False
        while True:
            if self._debug:
                print(self.state_dump(), file=self._debug)
            op = self._get_mem(self._PC)
            opcode = op % 100
            parmode1 = (op // 100) % 10
            parmode2 = (op // 1000) % 10
            parmode3 = (op // 10000) % 10
            if not (0 <= parmode1 <= 2):
                raise ValueError(
                    f"{parmode1} is not an appropriate parameter mode for parameter 1"
                )
            if not (0 <= parmode2 <= 2):
                raise ValueError(
                    f"{parmode2} is not an appropriate parameter mode for parameter 2"
                )
            if not (0 <= parmode3 <= 2):
                raise ValueError(
                    f"{parmode3} is not an appropriate parameter mode for parameter 3"
                )
            if opcode == 1:
                self._set_param(
                    self._PC + 3,
                    parmode3,
                    self._get_param(self._PC + 1, parmode1)
                    + self._get_param(self._PC + 2, parmode2),
                )
                self._PC += 4
            elif opcode == 2:
                self._set_param(
                    self._PC + 3,
                    parmode3,
                    self._get_param(self._PC + 1, parmode1)
                    * self._get_param(self._PC + 2, parmode2),
                )
                self._PC += 4
            elif opcode == 3:
                if not self._input:
                    return True  # wait for more input
                self._set_param(self._PC + 1, parmode1, self._input.pop(0))
                self._PC += 2
            elif opcode == 4:
                self._output.append(self._get_param(self._PC + 1, parmode1))
                self._PC += 2
            elif opcode == 5:
                if self._get_param(self._PC + 1, parmode1):
                    self._PC = self._get_param(self._PC + 2, parmode2)
                else:
                    self._PC += 3
            elif opcode == 6:
                if not self._get_param(self._PC + 1, parmode1):
                    self._PC = self._get_param(self._PC + 2, parmode2)
                else:
                    self._PC += 3
            elif opcode == 7:
                self._set_param(
                    self._PC + 3,
                    parmode3,
                    1
                    if (
                        self._get_param(self._PC + 1, parmode1)
                        < self._get_param(self._PC + 2, parmode2)
                    )
                    else 0,
                )
                self._PC += 4
            elif opcode == 8:
                self._set_param(
                    self._PC + 3,
                    parmode3,
                    1
                    if (
                        self._get_param(self._PC + 1, parmode1)
                        == self._get_param(self._PC + 2, parmode2)
                    )
                    else 0,
                )
                self._PC += 4
            elif opcode == 9:
                self._RB += self._get_param(self._PC + 1, parmode1)
                self._PC += 2
            elif opcode == 99:
                self._halted = True
                return False
            else:
                raise ValueError(f"Unknow opcode {opcode} at place {self._PC}")

    def give_input(self, inp: Union[int, Iterable[int]]) -> None:
        """Add input to the machine"""
        if isinstance(inp, int):
            self._input.append(inp)
        else:
            self._input.extend(inp)

    def get_output(self) -> Optional[int]:
        """Read output from the machine"""
        if not self._output:
            self.run()  # get more output
        if not self._output:
            return None
        return self._output.pop(0)

    @property
    def halted(self) -> bool:
        """Return if the machine stopped"""
        return self._halted


if __name__ == "__main__":
    from sys import argv, stdout

    program = tuple(
        int(x.strip()) for x in open(argv[1]).read().split(",") if x.strip()
    )
    machine = Machine(program, debug=open(argv[2], "w") if len(argv) > 2 else None)
    while machine.run():
        print(
            ", ".join(
                str(x)
                for x in takewhile(
                    lambda x: x is not None, (machine.get_output() for _ in count())
                )
            )
        )
        machine.give_input(int(input()))
    print(
        ", ".join(
            str(x)
            for x in takewhile(
                lambda x: x is not None, (machine.get_output() for _ in count())
            )
        )
    )
