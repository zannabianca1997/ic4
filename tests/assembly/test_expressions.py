from unittest import TestCase

from parameterized import parameterized_class, parameterized

from ic4.assembly.expressions import (
    Atom,
    Constant,
    Divide,
    Expression,
    Multiply,
    Reference,
    SimplifyException,
    Subtract,
    Sum,
)


class TestConstant(TestCase):
    constant: Constant
    value: int = 42

    def setUp(self) -> None:
        self.constant = Constant(self.value)

    def test_eredity(self):
        """Check that the constant is of the right types"""
        self.assertIsInstance(self.constant, Expression)
        self.assertIsInstance(self.constant, Atom)
        self.assertIsInstance(self.constant, Constant)

    def test_equality(self):
        """Check that it compares equal to similar expressions"""
        self.assertEqual(self.constant, Constant(self.value))
        self.assertEqual(self.constant, self.value)

    def test_hash(self):
        """Check that  hashes compares equal to similar expressions hashes"""
        self.assertEqual(hash(self.constant), hash(Constant(self.value)))
        self.assertEqual(hash(self.constant), hash(self.value))

    def test_inequality(self):
        """Check that it compares different to different expressions"""
        self.assertNotEqual(self.constant, Constant(self.value + 1))
        self.assertNotEqual(self.constant, self.value + 1)

    def test_simplify(self):
        """Check that it a simplification fixed point"""
        self.assertEqual(self.constant, self.constant.simplify())
        self.assertEqual(self.constant, self.constant.simplify(fullsimplify=False))


class TestReference(TestCase):
    constant: Constant
    value: str = "Answer"

    def setUp(self) -> None:
        self.constant = Reference(self.value)

    def test_eredity(self):
        """Check that the constant is of the right types"""
        self.assertIsInstance(self.constant, Expression)
        self.assertIsInstance(self.constant, Atom)
        self.assertIsInstance(self.constant, Reference)

    def test_equality(self):
        """Check that it compares equal to similar expressions"""
        self.assertEqual(self.constant, Reference(self.value))
        self.assertEqual(self.constant, self.value)

    def test_hash(self):
        """Check that  hashes compares equal to similar expressions hashes"""
        self.assertEqual(hash(self.constant), hash(Reference(self.value)))
        self.assertEqual(hash(self.constant), hash(self.value))

    def test_inequality(self):
        """Check that it compares different to different expressions"""
        self.assertNotEqual(self.constant, Reference(self.value + "a"))
        self.assertNotEqual(self.constant, self.value + "a")

    def test_simplify(self):
        """Check that it a simplification fixed point"""
        self.assertEqual(self.constant, self.constant.simplify(fullsimplify=False))
        self.assertRaises(SimplifyException, self.constant.simplify, fullsimplify=True)

        # check substitution
        self.assertEqual(
            Constant(42),
            self.constant.simplify(
                {Reference(self.value): Constant(42)}, fullsimplify=True
            ),
        )
        self.assertEqual(
            Constant(42),
            self.constant.simplify(
                {Reference(self.value): Constant(42)}, fullsimplify=False
            ),
        )
        self.assertEqual(
            Reference("question"),
            self.constant.simplify(
                {Reference(self.value): Reference("question")}, fullsimplify=False
            ),
        )

    def test_name_check(self):
        self.assertRaises(ValueError, Reference, "0.*")


@parameterized_class(
    [
        {
            "name": "Sum",
            "cls": Sum,
            "commutative": True,
            "fun": lambda self, x, y: x + y,
        },
        {
            "name": "Subtract",
            "cls": Subtract,
            "commutative": False,
            "fun": lambda self, x, y: x - y,
        },
        {
            "name": "Multiply",
            "cls": Multiply,
            "commutative": True,
            "fun": lambda self, x, y: x * y,
        },
        {
            "name": "Divide",
            "cls": Divide,
            "commutative": False,
            "fun": lambda self, x, y: x // y,
        },
    ]
)
class TestCommonBinOp(TestCase):
    def test_commutativity(self):
        if not self.commutative:
            self.skipTest(f"Operation {self.name} is not commutative")
        self.assertEqual(
            self.cls(Constant(23), Constant(45)),
            self.cls(Constant(45), Constant(23)),
        )
        self.assertEqual(
            self.cls(Reference("a"), Constant(45)),
            self.cls(Constant(45), Reference("a")),
        )

    def test_simplify(self):
        self.assertEqual(
            self.cls(Constant(23), Constant(45)).simplify(),
            self.fun(23, 45),
        )
        self.assertRaises(
            SimplifyException,
            self.cls(Reference("a"), Constant(45)).simplify,
            fullsimplify=True,
        )
        self.assertEqual(
            self.cls(Reference("a"), Constant(45)).simplify(
                {Reference("a"): Constant(23)},
                fullsimplify=True,
            ),
            self.fun(23, 45),
        )


class TestNullElements(TestCase):
    def test_Sum(self):
        self.assertEqual(
            Sum(Reference("a"), Constant(0)).simplify(fullsimplify=False),
            Reference("a"),
        )
        self.assertEqual(
            Sum(Constant(0), Reference("a")).simplify(fullsimplify=False),
            Reference("a"),
        )

    def test_Subtract(self):
        self.assertEqual(
            Subtract(Reference("a"), Constant(0)).simplify(fullsimplify=False),
            Reference("a"),
        )

    def test_Multiply(self):
        self.assertEqual(
            Multiply(Reference("a"), Constant(1)).simplify(fullsimplify=False),
            Reference("a"),
        )
        self.assertEqual(
            Multiply(Constant(1), Reference("a")).simplify(fullsimplify=False),
            Reference("a"),
        )

    def test_Divide(self):
        self.assertEqual(
            Divide(Reference("a"), Constant(1)).simplify(fullsimplify=False),
            Reference("a"),
        )


class TestAdsorbElements(TestCase):
    def test_Multiply(self):
        self.assertEqual(
            Multiply(Reference("a"), Constant(0)).simplify(fullsimplify=False),
            Constant(0),
        )
        self.assertEqual(
            Multiply(Constant(0), Reference("a")).simplify(fullsimplify=False),
            Constant(0),
        )

    def test_Divide(self):
        self.assertEqual(
            Divide(Constant(0), Reference("a")).simplify(fullsimplify=False),
            Constant(0),
        )


class TestSpecialCase(TestCase):
    def test_Multiply(self):
        self.assertEqual(
            Multiply(Reference("a"), Constant(0)).simplify(fullsimplify=False),
            Constant(0),
        )
        self.assertEqual(
            Multiply(Constant(0), Reference("a")).simplify(fullsimplify=False),
            Constant(0),
        )

    def test_Divide(self):
        self.assertEqual(
            Divide(Reference("a"), Constant(-1)).simplify(fullsimplify=False),
            Multiply(Reference("a"), Constant(-1)),
        )
        self.assertRaises(SimplifyException, Divide(Constant(0), Constant(0)).simplify)
        self.assertRaises(SimplifyException, Divide(Constant(1), Constant(0)).simplify)


class TestOperators(TestCase):
    OPS = [
        ("sum", Sum, lambda a, b: a + b),
        ("mul", Multiply, lambda a, b: a * b),
        ("div", Divide, lambda a, b: a // b),
        ("sub", Subtract, lambda a, b: a - b),
    ]

    def setUp(self) -> None:
        self.a = Reference("a")
        self.b = Reference("b")
        self.three = Constant(3)
        self.four = Constant(4)

    @parameterized.expand(OPS)
    def test_references(self, name, cls, fun):
        self.assertEqual(cls(self.a, self.b), fun(self.a, self.b))

    @parameterized.expand(OPS)
    def test_constant(self, name, cls, fun):
        self.assertEqual(cls(self.three, self.four), fun(self.three, self.four))

    @parameterized.expand(OPS)
    def test_constant_and_refs(self, name, cls, fun):
        self.assertEqual(cls(self.a, self.four), fun(self.a, self.four))
        self.assertEqual(cls(self.four, self.a), fun(self.four, self.a))

    @parameterized.expand(OPS)
    def test_reference_and_lit(self, name, cls, fun):
        self.assertEqual(cls(self.a, self.three), fun(self.a, 3))
        self.assertEqual(cls(self.three, self.a), fun(3, self.a))

    @parameterized.expand(OPS)
    def test_constant_and_lit(self, name, cls, fun):
        self.assertEqual(cls(self.three, self.four), fun(self.three, 4))
        self.assertEqual(cls(self.four, self.three), fun(4, self.three))

    @parameterized.expand(OPS)
    def test_reference_and_str(self, name, cls, fun):
        self.assertEqual(cls(self.a, self.b), fun(self.a, "b"))
        self.assertEqual(cls(self.b, self.a), fun("b", self.a))

    @parameterized.expand(OPS)
    def test_constant_and_str(self, name, cls, fun):
        self.assertEqual(cls(self.three, self.b), fun(self.three, "b"))
        self.assertEqual(cls(self.b, self.three), fun("b", self.three))
