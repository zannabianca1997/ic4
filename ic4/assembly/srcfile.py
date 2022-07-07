from dataclasses import dataclass
from typing import FrozenSet, Optional, Tuple, Union

from .commands import Command
from ..version import Version


@dataclass(frozen=True)
class ExecutableHeader:
    version: Version

    def __str__(self) -> str:
        return f"EXECUTABLE {self.version}\n"


@dataclass(frozen=True)
class ObjectsHeader:
    version: Version
    export: FrozenSet[str] = frozenset()
    extern: FrozenSet[str] = frozenset()
    entry: Optional[str] = None

    def __str__(self) -> str:
        return (
            f"EXECUTABLE {self.version}\n"
            + (f"EXPORT {' '.join(self.export)}\n" if self.export else "")
            + (f"EXTERN {' '.join(self.extern)}\n" if self.extern else "")
            + (f"ENTRY {self.entry}\n" if self.entry is not None else "")
        )


@dataclass(frozen=True)
class SourceFile:
    header: Union[ExecutableHeader, ObjectsHeader]
    body: Tuple[Command, ...]

    @property
    def is_executable(self):
        return isinstance(self.header, ExecutableHeader)

    @property
    def is_objects(self):
        return isinstance(self.header, ObjectsHeader)

    def __str__(self) -> str:
        return str(self.header) + "".join(str(c) for c in self.body)
