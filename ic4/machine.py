"""
    Implement a IntCode machine
"""

from typing import Union, Callable, Iterable, Optional, Tuple, List


class Machine:
    """A IntCode machine"""
    _memory: List[int]
    _PC: int
    _RB: int
    _halted: bool

    _input: List[int]
    _output: List[int]

    _input_fun: Optional[Callable[[], int]]
    _output_fun: Optional[Callable[[int], bool]]

    def __init__(self,
                 program: Tuple[int]) -> None:
        self._memory = list(program)
        self._PC = 0
        self._RB = 0
        self._halted = False

        self._input = []
        self._output = []

    def _get_mem(self, idx: int, parmode: int) -> int:
        """Get memory at index"""
        if parmode == 1:
            return idx  # immediate mode
        if parmode == 2:
            idx += self._RB
        if idx < 0:
            raise IndexError(
                f"Accessed negative memory location {idx} for reading")
        if len(self._memory) <= idx:  # memory is shorter than what we want
            return 0
        else:
            return self._memory[idx]

    def _set_mem(self, idx: int, parmode: int, val: int) -> None:
        """Set memory at a index"""
        if parmode == 1:
            raise ValueError(f"Cannot use immediate mode for writing")
        if parmode == 2:
            idx += self._RB
        if idx < 0:
            raise IndexError(
                f"Accessed negative memory location {idx} for writing")
        if len(self._memory) <= idx:  # memory is shorter than what we need
            if val != 0:
                self._memory += [0] * \
                    (idx + 1 - len(self._memory))  # pad the list
                self._memory[idx] = val
        else:
            self._memory[idx] = val

    def run(self) -> bool:
        """Run the machine until halted or input is needed. return True if not halted"""
        if self._halted:
            return False
        while True:
            op = self._get_mem(self._PC, 0)
            opcode = op % 100
            parmode1 = (op // 100) % 10
            parmode2 = (op // 1000) % 10
            parmode3 = (op // 10000) % 10
            if not (0 <= parmode1 <= 2):
                raise ValueError(
                    f"{parmode1} is not an appropriate parameter mode for parameter 1")
            if not (0 <= parmode2 <= 2):
                raise ValueError(
                    f"{parmode2} is not an appropriate parameter mode for parameter 2")
            if not (0 <= parmode3 <= 2):
                raise ValueError(
                    f"{parmode3} is not an appropriate parameter mode for parameter 3")
            if opcode == 1:
                self._set_mem(
                    self._PC + 3, parmode3,
                    self._get_mem(self._PC+1, parmode1) +
                    self._get_mem(self._PC+2, parmode2)
                )
                self._PC += 4
            elif opcode == 2:
                self._set_mem(
                    self._PC + 3, parmode3,
                    self._get_mem(self._PC+1, parmode1) *
                    self._get_mem(self._PC+2, parmode2)
                )
                self._PC += 4
            elif opcode == 3:
                if not self._input:
                    return True  # wait for more input
                self._set_mem(self._PC+1, parmode1, self._input.pop(0))
                self._PC += 2
            elif opcode == 4:
                self._output.append(
                    self._get_mem(self._PC+1, parmode1))
                self._PC += 2
            elif opcode == 5:
                if self._get_mem(self._PC+1, parmode1):
                    self._PC = self._get_mem(self._PC+2, parmode2)
                else:
                    self._PC += 3
            elif opcode == 6:
                if not self._get_mem(self._PC+1, parmode1):
                    self._PC = self._get_mem(self._PC+2, parmode2)
                else:
                    self._PC += 3
            elif opcode == 7:
                self._set_mem(
                    self._PC + 3, parmode3,
                    1 if (self._get_mem(self._PC+1, parmode1) <
                          self._get_mem(self._PC+2, parmode2)) else 0
                )
                self._PC += 4
            elif opcode == 8:
                self._set_mem(
                    self._PC + 3, parmode3,
                    1 if (self._get_mem(self._PC+1, parmode1) ==
                          self._get_mem(self._PC+2, parmode2)) else 0
                )
                self._PC += 4
            elif opcode == 9:
                self._RB += self._get_mem(self._PC+1, parmode1)
                self._PC += 2
            elif opcode == 99:
                self._halted = True
                return False
            else:
                raise ValueError(f"Unknow opcode {opcode}")

    def give_input(self, inp: Union[int, Iterable[int]]) -> None:
        """Add input to the machine"""
        if isinstance(input, int):
            self._input.append(input)
        else:
            self._input.extend(input)

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
