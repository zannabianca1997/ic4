from dataclasses import dataclass
from typing import FrozenSet, Optional, Tuple, Union

from .commands import Command
from ..version import Version


@dataclass(frozen=True)
class ExecutableHeader:
    version: Version


@dataclass(frozen=True)
class ObjectsHeader:
    version: Version
    export: FrozenSet[str] = frozenset()
    extern: FrozenSet[str] = frozenset()
    entry: Optional[str] = None


@dataclass(frozen=True)
class SourceFile:
    header: Union[ExecutableHeader, ObjectsHeader]
    body: Tuple[Command]
