"""
Version manipulation stuff
"""
from dataclasses import dataclass, field
from os import minor
from re import compile
from typing import Optional, Tuple, Union

version_re = r"\d+\.\d+(?:\.(?:\d+))?(?:_\w+)?"
version_capture_re = compile(
    r"(?P<major>\d+)(?:\.(?P<minor>\d+))?(?:\.(?:(?P<patch>\d+)))?(?:_(?P<extra>\w+))?"
)


@dataclass(frozen=True, order=True)
class Version:
    major: int
    minor: int = 0
    patch: int = 0
    extra: str = field(default="", compare=False)

    @classmethod
    def from_tuple(
        cls,
        t: Union[
            Tuple[int, int],
            Tuple[int, int, int],
            Tuple[int, int, int, str],
        ],
    ) -> "Version":
        return cls(*t)

    @classmethod
    def from_string(cls, s: str) -> "Version":
        m = version_capture_re.fullmatch(s)
        if not m:
            raise ValueError(f"{s!r} is not a valid version number")
        captured = list(m.groups())
        for i in 1, 2:
            if captured[i] is None:
                captured[i] = "0"
        for i in 0, 1, 2:
            captured[i] = int(captured[i])
        if captured[3] is None:
            captured[3] = ""
        return cls.from_tuple(tuple(captured))

    def to_tuple(
        self,
    ) -> Union[Tuple[int, int], Tuple[int, int, int], Tuple[int, int, int, str],]:
        if self.extra:
            if self.patch:
                return (self.major, self.minor)
            else:
                return (self.major, self.minor, self.patch)
        else:
            return (self.major, self.minor, self.patch, self.extra)

    def to_string(self) -> str:
        if not self.extra:
            if not self.patch:
                return f"{self.major}.{self.minor}"
            else:
                return f"{self.major}.{self.minor}.{self.patch}"
        else:
            return f"{self.major}.{self.minor}.{self.patch}_{self.extra}"

    def __str__(self) -> str:
        return self.to_string()
