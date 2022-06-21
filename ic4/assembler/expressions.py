"""
    Methods to manipulate arithmetic expressions
"""
from dataclasses import dataclass
from typing import Dict, Optional, Tuple, Union

# A single, atomic factor: a number, a identifier,
# or a parentisized expression.
Factor = Union[int, str, "Expression"]


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


class SimplifyException(Exception):
    pass


def simplify(
    expr: Expression,
    subs: Optional[Dict[str, Expression]] = None,
    full_simplify: bool = False,
) -> Expression:
    """
    Return a simplified copy of an expression.
    If no references are inside it, it will get down to a single int.
    Can be given a dictionary of substitutions to reduce the references.
    If full_simplify == True the process will halt and throw a SimplifyException if it can't get
    the expression down to a single int.
    """
    if subs is None:
        subs = {}
    if isinstance(expr, int):
        return expr  # cannot be simplier
    if isinstance(expr, str):
        if expr in subs:
            # simplify that expression and then return that
            return simplify(subs[expr], subs)
        if full_simplify:
            raise SimplifyException(
                f"Cannot find '{expr}' within the substitution given ({subs})"
            )
        return expr

    # it's one of the four binary operations
    left = simplify(expr.left, subs)
    right = simplify(expr.right, subs)

    if not (isinstance(left, int) and isinstance(right, int)):
        assert (
            not full_simplify
        ), "When fullsimplifyin an exception should be thrown before there"
        return expr.__class__(left, right)  # simplify

    if isinstance(expr, Sum):
        return left + right
    if isinstance(expr, Subtract):
        return left - right
    if isinstance(expr, Multiply):
        return left * right
    if isinstance(expr, Divide):
        return left // right  # integer division only

    assert False, f"Type {type(expr)} should not be in an expression."
