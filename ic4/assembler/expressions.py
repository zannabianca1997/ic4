"""
    Methods to manipulate arithmetic expressions
"""
from dataclasses import dataclass
from typing import Dict, Iterable, Optional, Tuple, Union

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


def simplify_tuple(
    exprs: Iterable[Expression],
    subs: Optional[Dict[str, Expression]] = None,
    full_simplify: bool = False,
) -> Tuple[Expression, ...]:
    return tuple(simplify(val, subs, full_simplify=full_simplify) for val in exprs)


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
            return simplify(subs[expr], subs, full_simplify=full_simplify)
        if full_simplify:
            raise SimplifyException(
                f"Cannot find '{expr}' within the substitution given ({subs})"
            )
        return expr

    # it's one of the four binary operations
    left = simplify(expr.left, subs, full_simplify=full_simplify)
    right = simplify(expr.right, subs, full_simplify=full_simplify)

    # arithmetic identities
    if isinstance(expr, Sum):
        if left == 0:
            return right
        if right == 0:
            return left
    if isinstance(expr, Subtract):
        if right == 0:
            return left
    if isinstance(expr, Multiply):
        if left == 0 or right == 0:
            return 0
        if left == 1:
            return right
        if right == 1:
            return left
    if isinstance(expr, Divide):
        if left == 0:
            return 0
        if right == 1:
            return left

    if not (isinstance(left, int) and isinstance(right, int)):
        assert (
            not full_simplify
        ), "When fullsimplifyin an exception should be thrown before there"
        return expr.__class__(left, right)  # simplified version

    if isinstance(expr, Sum):
        return left + right
    if isinstance(expr, Subtract):
        return left - right
    if isinstance(expr, Multiply):
        return left * right
    if isinstance(expr, Divide):
        return left // right  # integer division only

    assert False, f"Type {type(expr)} should not be in an expression."
