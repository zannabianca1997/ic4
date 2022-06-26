"""
Parse an assembly file
"""
from sly import Parser


from .expressions import Divide, Multiply, Subtract, Sum
from .commands import Instruction, OpCode, ParamMode
from .lexer import ICAssLexer


class ICAssParser(Parser):
    tokens = ICAssLexer.tokens

    precedence = (
        ("left", PLUS, MINUS),
        ("left", TIMES, DIVIDE),
        ("right", UMINUS),  # Unary minus operator
    )

    @_("OPCODE { param [ COMMA ] }")
    def instruction(self, p):
        return Instruction(OpCode[p.OPCODE], tuple(p.param))

    @_("param_mode expr")
    def param(self, p):
        return (p.param_mode, p.expr)

    @_("")
    def param_mode(self, p):
        return ParamMode.MODE0

    @_("IMMEDIATE")
    def param_mode(self, p):
        return ParamMode.MODE1

    @_("RELATIVE")
    def param_mode(self, p):
        return ParamMode.MODE2

    @_("expr PLUS expr")
    def expr(self, p):
        return Sum(p.expr0, p.expr1)

    @_("expr MINUS expr")
    def expr(self, p):
        return Subtract(p.expr0, p.expr1)

    @_("expr TIMES expr")
    def expr(self, p):
        return Multiply(p.expr0, p.expr1)

    @_("expr DIVIDE expr")
    def expr(self, p):
        return Divide(p.expr0, p.expr1)

    @_("MINUS expr %prec UMINUS")
    def expr(self, p):
        return Multiply(p.expr, -1)

    @_("NUMBER")
    def expr(self, p):
        return p.NUMBER

    @_("IDENTIFIER")
    def expr(self, p):
        return p.IDENTIFIER

    @_("LPAREN expr RPAREN")
    def expr(self, p):
        return p.expr
