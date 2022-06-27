"""
Parse an assembly file
"""
from itertools import chain
from os import getenv
from pathlib import Path
from sly import Parser


from .expressions import Divide, Multiply, Subtract, Sum
from .commands import Directive, DirectiveCode, Instruction, Label, OpCode, ParamMode
from .lexer import ICAssLexer


class ICAssParser(Parser):
    tokens = ICAssLexer.tokens

    debugfile = Path(getenv("LOG_DIR")) / "parser.out"

    precedence = (
        ("left", PLUS, MINUS),
        ("left", TIMES, DIVIDE),
        ("right", UMINUS),  # Unary minus operator
    )

    @_("{ command LINE_END }")
    def program(self, p):
        return tuple(chain(*p.command))

    @_("labels instruction", "labels directive")
    def command(self, p):
        return (*p.labels, p[1])

    @_("labels")
    def command(self, p):
        return p.labels

    @_("{ IDENTIFIER COLON }")
    def labels(self, p):
        return tuple(Label(x) for x in p.IDENTIFIER)

    @_("OPCODE { param [ COMMA ] }")
    def instruction(self, p):
        return Instruction(OpCode[p.OPCODE], tuple(p.param))

    # directives with expression parameters
    @_("INTS { expr [ COMMA ] }")
    def directive(self, p):
        return Directive(DirectiveCode[p[0]], tuple(p.expr))

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