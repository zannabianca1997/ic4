from dataclasses import dataclass
from typing import Optional, Tuple, Union

from .commands import Command
from ..version import Version


@dataclass(frozen=True)
class ExecutableHeader:
    version: Version


@dataclass(frozen=True)
class ObjectsHeader:
    version: Version
    export: Tuple[str, ...]
    extern: Tuple[str, ...]
    entry: Optional[str]


@dataclass(frozen=True)
class SourceFile:
    header: Union[ExecutableHeader, ObjectsHeader]
    body: Tuple[Command]
