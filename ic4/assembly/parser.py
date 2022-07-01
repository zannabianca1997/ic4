"""
Parse an assembly file
"""
from itertools import chain
from os import getenv
from pathlib import Path
from sly import Parser
from ic4.assembly.srcfile import ExecutableHeader, SourceFile

from ic4.string_utilities import unescape_string_const


from .expressions import Divide, Multiply, Subtract, Sum
from .commands import Directive, DirectiveCode, Instruction, Label, OpCode, ParamMode
from .lexer import ICAssLexer

Path(getenv("LOG_DIR")).mkdir(parents=True, exist_ok=True)  # creating log dir


class ICAssParser(Parser):
    tokens = ICAssLexer.tokens

    debugfile = Path(getenv("LOG_DIR")) / "parser.out"

    precedence = (
        ("left", PLUS, MINUS),
        ("left", TIMES, DIVIDE),
        ("right", UMINUS),  # Unary minus operator
    )

    @_("header { command lline_end }")
    def program(self, p):
        return SourceFile(p.header, tuple(chain(*p.command)))

    @_("EXECUTABLE VERSION lline_end")
    def header(self, p):
        return ExecutableHeader(p.VERSION)

    # grouping together line ends
    @_("LINE_END { LINE_END }")
    def lline_end(self, p):
        pass

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

    # --- directives ---

    @_("INTS { expr [ COMMA ] }")
    def directive(self, p):
        return Directive(DirectiveCode[p[0]], tuple(p.expr))

    @_("INTS STRING")
    def directive(self, p):
        return Directive(DirectiveCode[p[0]], p.STRING)

    @_("ZEROS expr")
    def directive(self, p):
        return Directive(DirectiveCode[p[0]], (p.expr,))

    @_("INC param", "DEC param")
    def directive(self, p):
        return Directive(DirectiveCode[p[0]], (p.param,))

    @_("MOV param [ COMMA ] param [ COMMA ] [ expr ]")
    def directive(self, p):
        return Directive(
            DirectiveCode[p[0]],
            (p.param0, p.param1, p.expr or 1),
        )

    @_("LOAD param [ COMMA ] param", "STORE param [ COMMA ] param")
    def directive(self, p):
        return Directive(DirectiveCode[p[0]], (p.param0, p.param1))

    @_("JMP param")
    def directive(self, p):
        return Directive(DirectiveCode[p[0]], (p.param,))

    @_("PUSH [ param ] [ COMMA ] [ expr ]")
    def directive(self, p):
        return Directive(DirectiveCode[p[0]], (p.param, p.expr or 1))

    @_("POP [ param ] [ COMMA ] [ expr ]")
    def directive(self, p):
        return Directive(DirectiveCode[p[0]], (p.param, p.expr or 1))

    @_("CALL param")
    def directive(self, p):
        return Directive(DirectiveCode[p[0]], (p.param,))

    @_("RET")
    def directive(self, p):
        return Directive(DirectiveCode[p[0]], ())

    # --- parameters ---

    @_("param_mode expr")
    def param(self, p):
        return (p.param_mode, p.expr)

    @_("")
    def param_mode(self, p):
        return ParamMode.ABSOLUTE

    @_("IMMEDIATE")
    def param_mode(self, p):
        return ParamMode.IMMEDIATE

    @_("RELATIVE")
    def param_mode(self, p):
        return ParamMode.RELATIVE

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
