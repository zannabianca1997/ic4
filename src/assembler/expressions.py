"""
    Methods to manipulate arithmetic expressions
"""
from dataclasses import dataclass
from typing import Tuple, Union

# A single, atomic factor: a number, a identifier,
# or a parentisized expression.
Factor = Union[int, str, 'Expression']

# A product of factors
Term = Tuple[
    Tuple[bool, Factor],  # first element is true if it's multiplied
    ...
]

# A sum of terms
Expression = Tuple[
    Tuple[bool, Term],  # first element is true if it's divided
    ...
]
