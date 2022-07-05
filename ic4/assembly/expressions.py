"""
    Methods to manipulate arithmetic expressions
"""
from operator import index
from sys import stderr
from typing import AnyStr, Dict, Optional, SupportsInt, Union
from re import compile, Pattern


class SimplifyException(Exception):
    """Raised if a simplifying did not complete"""

    pass


class Expression:
    """A symbolic arithmetic expression"""

    __slots__ = ()

    def __init__(self) -> None:
        raise NotImplementedError()

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

    # operators
    def __add__(self, __o: object) -> "Sum":
        if not isinstance(__o, (Expression, SupportsInt, str)):
            return NotImplemented
        return Sum(self, __o if isinstance(__o, Expression) else Atom.fromval(__o))

    def __radd__(self, __o: object) -> "Sum":
        if not isinstance(__o, (Expression, SupportsInt, str)):
            return NotImplemented
        return Sum(self, __o if isinstance(__o, Expression) else Atom.fromval(__o))

    def __sub__(self, __o: object) -> "Subtract":
        if not isinstance(__o, (Expression, SupportsInt, str)):
            return NotImplemented
        return Subtract(self, __o if isinstance(__o, Expression) else Atom.fromval(__o))

    def __rsub__(self, __o: object) -> "Subtract":
        if not isinstance(__o, (Expression, SupportsInt, str)):
            return NotImplemented
        return Subtract(__o if isinstance(__o, Expression) else Atom.fromval(__o), self)

    def __mul__(self, __o: object) -> "Multiply":
        if not isinstance(__o, (Expression, SupportsInt, str)):
            return NotImplemented
        return Multiply(self, __o if isinstance(__o, Expression) else Atom.fromval(__o))

    def __rmul__(self, __o: object) -> "Multiply":
        if not isinstance(__o, (Expression, SupportsInt, str)):
            return NotImplemented
        return Multiply(self, __o if isinstance(__o, Expression) else Atom.fromval(__o))

    def __floordiv__(self, __o: object) -> "Divide":
        if not isinstance(__o, (Expression, SupportsInt, str)):
            return NotImplemented
        return Divide(self, __o if isinstance(__o, Expression) else Atom.fromval(__o))

    def __rfloordiv__(self, __o: object) -> "Divide":
        if not isinstance(__o, (Expression, SupportsInt, str)):
            return NotImplemented
        return Divide(__o if isinstance(__o, Expression) else Atom.fromval(__o), self)

    def __neg__(self) -> "Multiply":
        return Multiply(self, Constant(-1))

    def __pos__(self) -> "Expression":
        return self

    def __int__(self) -> int:
        return index(self)

    def __index__(self) -> int:
        return int(self.simplify(fullsimplify=True))


class Atom(Expression):
    """An atomic value"""

    __slots__ = ("value",)

    def __str__(self) -> str:
        return str(self.value)

    def __repr__(self) -> str:
        return f"{self.__class__.__name__}({self.value!r})"

    @staticmethod
    def fromval(value: Union[SupportsInt, AnyStr]) -> Union["Constant", "Reference"]:
        if isinstance(value, SupportsInt):
            return Constant(value)
        elif isinstance(value, str):
            return Reference(value)
        raise TypeError()


class Constant(Atom):
    """A number"""

    __slots__ = ()
    value: int

    def __init__(self, value: Union[SupportsInt, str]) -> None:
        self.value = int(value)

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

    def __index__(self) -> int:
        return self.value


class Reference(Atom):
    """A reference (to a variable, or a label, etc)"""

    __slots__ = ()
    value: str

    INTERNAL_NAMES_RE: Pattern = compile(
        r"[_a-zA-Z&][_a-zA-Z0-9$]*"
    )  # names that a reference can have
    NAMES_RE: str = r"[_a-zA-Z][_a-zA-Z0-9$]*"  # names usable by the assembler ( &-starting names are reserved )

    def __init__(self, value: AnyStr) -> None:
        value = str(value)
        if not self.INTERNAL_NAMES_RE.fullmatch(value):
            raise ValueError(f"{value!r} is not an appropriate name for a reference")
        self.value = value

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


class Sum(BinOp):
    """Sum of two expressions"""

    __slots__ = ()

    def __init__(self, left, right) -> None:
        self.left = left
        self.right = right

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

    def __init__(self, left, right) -> None:
        self.left = left
        self.right = right

    def simplify(
        self,
        subs: Optional[Dict["Reference", "Expression"]] = None,
        fullsimplify: bool = True,
    ) -> "Expression":
        left = self.left.simplify(subs, fullsimplify)
        # morph into multiplication
        if left == 0:
            return Multiply(self.right, Constant(-1)).simplify()

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

    def __init__(self, left, right) -> None:
        self.left = left
        self.right = right

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

    def __init__(self, left, right) -> None:
        self.left = left
        self.right = right

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
