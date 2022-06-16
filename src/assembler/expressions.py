"""
    Methods to manipulate arithmetic expressions
"""
from ast import Expression
from dataclasses import dataclass
from typing import Tuple, Union

# A single, atomic factor: a number, a identifier,
# or a parentisized expression.
Factor = Union[int, str, 'Expression']


@dataclass(frozen=True)
class Multiply:
    left: Factor
    right: Factor


@dataclass(frozen=True)
class Divide:
    left: Factor
    right: Factor


Term = Union[Multiply, Divide, Factor]


@dataclass(frozen=True)
class Sum:
    left: Factor
    right: Factor


@dataclass(frozen=True)
class Subtract:
    left: Factor
    right: Factor


Expression = Union[Sum, Subtract, Term]
