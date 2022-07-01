"""
    Methods to manipulate arithmetic expressions
"""
from typing import Dict, Optional, SupportsInt, Union


class SimplifyException(Exception):
    """Raised if a simplifying did not complete"""

    pass


class Expression:
    """A symbolic arithmetic expression"""

    __slots__ = ()

    def simplify(
        self,
        subs: Optional[Dict["Reference", "Expression"]] = None,
        fullsimplify: bool = True,
    ) -> "Expression":
        """Simplify this expression

        :param subs: The substitutions to use while simplifying, defaults to None
        :type subs: Optional[Dict[Reference, Expression]], optional
        :param fullsimplify: If True, the method will return a Constant, defaults to True
        :type fullsimplify: bool, optional
        :raises SimplifyException: If fullsimplify == True and a constant is unreachable
        :return: The simplified expression
        :rtype: Expression
        """
        if fullsimplify:
            raise NotImplementedError(f"simplify not implemented for {self.__class__}")
        return self

    def __str__(self) -> str:
        """Return a human readable form of the expression

        :return: the expression in string form
        :rtype: str
        """
        raise NotImplementedError(f"__str__ not implemented for {self.__class__}")

    def __eq__(self, __o: object) -> bool:
        """Compare expression for equality

        :param __o: the other expression
        :type __o: object
        :return: True if equal, False otherwise
        :rtype: bool
        """
        raise NotImplementedError(f"__eq__ not implemented for {self.__class__}")

    def __hash__(self) -> int:
        """hash of the expression.

        :return: the hash
        :rtype: int
        """
        raise NotImplementedError(f"__hash__ not implemented for {self.__class__}")


class Atom(Expression):
    """An atomic value"""

    __slots__ = ("value",)

    def __init__(self, value: Union[SupportsInt, str]) -> None:
        super().__init__()
        self.value = value

    def __str__(self) -> str:
        return str(self.value)

    def __repr__(self) -> str:
        return f"{self.__class__.__name__}({self.value!r})"


class Constant(Atom):
    """A number"""

    __slots__ = ()
    value: int

    def __init__(self, value: Union[SupportsInt, str]) -> None:
        super().__init__(int(value))

    def simplify(
        self,
        subs: Optional[Dict["Reference", "Expression"]] = None,
        fullsimplify: bool = True,
    ) -> "Constant":
        return self  # no need to go further

    def __eq__(self, __o: object) -> bool:
        # try simplify the other
        if isinstance(__o, Constant):
            return self.value == __o.value
        # compare with ints
        if isinstance(__o, SupportsInt):
            return self.value == int(__o)
        return False

    def __hash__(self) -> int:
        return hash(self.value)


class Reference(Atom):
    """A reference (to a variable, or a label, etc)"""

    __slots__ = ()
    value: str

    def __init__(self, value: str) -> None:
        super().__init__(str(value))

    def simplify(
        self,
        subs: Optional[Dict["Reference", "Expression"]] = None,
        fullsimplify: bool = True,
    ) -> Expression:
        if subs is None or self not in subs:
            if fullsimplify:
                raise SimplifyException(
                    f"{self.value!r} not in the given substitutions!"
                )
            else:
                return self
        return subs[self].simplify(subs, fullsimplify)

    def __eq__(self, __o: object) -> bool:
        # try simplify the other
        if isinstance(__o, Reference):
            return self.value == __o.value
        # compare with ints
        if isinstance(__o, str):
            return self.value == __o
        return False

    def __hash__(self) -> int:
        return hash(self.value)


class BinOp(Expression):
    __slots__ = ("left", "right")
    left: Expression
    right: Expression

    def __init__(self, left, right) -> None:
        super().__init__()
        self.left = left
        self.right = right


class Sum(BinOp):
    """Sum of two expressions"""

    __slots__ = ()

    def simplify(
        self,
        subs: Optional[Dict["Reference", "Expression"]] = None,
        fullsimplify: bool = True,
    ) -> "Expression":
        left = self.left.simplify(subs, fullsimplify)
        right = self.right.simplify(subs, fullsimplify)

        # sum by zero
        if left == 0:
            return right
        if right == 0:
            return left

        # sum of constants
        if isinstance(left, Constant) and isinstance(right, Constant):
            return Constant(left.value + right.value)

        return Sum(left, right)

    def __str__(self) -> str:
        return f"({self.left}) + ({self.right})"

    def __eq__(self, __o: object) -> bool:
        if not isinstance(__o, Sum):
            return False
        # using sets so a+b == b+a
        return frozenset((self.left, self.right)) == frozenset((__o.left, __o.right))

    def __hash__(self) -> int:
        return hash(("+", frozenset((self.left, self.right))))


class Subtract(BinOp):
    """Difference of two expressions"""

    __slots__ = ()

    def simplify(
        self,
        subs: Optional[Dict["Reference", "Expression"]] = None,
        fullsimplify: bool = True,
    ) -> "Expression":
        left = self.left.simplify(subs, fullsimplify)
        # morph into multiplication
        if left == 0:
            return Multiply(right, Constant(-1)).simplify()

        right = self.right.simplify(subs, fullsimplify)
        # subtract 0
        if right == 0:
            return left

        # difference of constants
        if isinstance(left, Constant) and isinstance(right, Constant):
            return Constant(left.value - right.value)

        return Subtract(left, right)

    def __str__(self) -> str:
        return f"({self.left}) - ({self.right})"

    def __eq__(self, __o: object) -> bool:
        if not isinstance(__o, Subtract):
            return False
        # using tuples so a-b != b-a
        return tuple((self.left, self.right)) == tuple((__o.left, __o.right))

    def __hash__(self) -> int:
        return hash(("-", tuple((self.left, self.right))))


class Multiply(BinOp):
    """Product of two expressions"""

    __slots__ = ()

    def simplify(
        self,
        subs: Optional[Dict["Reference", "Expression"]] = None,
        fullsimplify: bool = True,
    ) -> "Expression":
        left = self.left.simplify(subs, fullsimplify)
        # multiply by zero
        if left == 0:
            return Constant(0)
        right = self.right.simplify(subs, fullsimplify)
        # multiply by zero
        if right == 0:
            return Constant(0)

        # multiply by one
        if left == 1:
            return right
        if right == 1:
            return left

        # product of constants
        if isinstance(left, Constant) and isinstance(right, Constant):
            return Constant(left.value * right.value)

        return Multiply(left, right)

    def __str__(self) -> str:
        return f"({self.left}) * ({self.right})"

    def __eq__(self, __o: object) -> bool:
        if not isinstance(__o, Multiply):
            return False
        # using sets so a*b == b*a
        return frozenset((self.left, self.right)) == frozenset((__o.left, __o.right))

    def __hash__(self) -> int:
        return hash(("*", frozenset((self.left, self.right))))


class Divide(BinOp):
    """Integer division of two expressions"""

    __slots__ = ()

    def simplify(
        self,
        subs: Optional[Dict["Reference", "Expression"]] = None,
        fullsimplify: bool = True,
    ) -> "Expression":
        right = self.right.simplify(subs, fullsimplify)
        if right == 1:
            return self.left.simplify(subs, fullsimplify)
        if right == -1:  # morph into a multiplication
            return Multiply(self.left, Constant(-1)).simplify(subs, fullsimplify)

        left = self.left.simplify(subs, fullsimplify)

        if left == 0:
            if right == 0:
                raise SimplifyException("Indeterminate form 0//0 encountered")
            return Constant(0)
        if isinstance(left, Constant) and right == 0:
            raise SimplifyException("Division by 0 encountered")

        # division of constants
        if isinstance(left, Constant) and isinstance(right, Constant):
            return Constant(left.value // right.value)

        return Divide(left, right)

    def __str__(self) -> str:
        return f"({self.left}) / ({self.right})"

    def __eq__(self, __o: object) -> bool:
        if not isinstance(__o, Divide):
            return False
        # using tuples so a/b != b/a
        return tuple((self.left, self.right)) == tuple((__o.left, __o.right))

    def __hash__(self) -> int:
        return hash(("/", tuple((self.left, self.right))))
