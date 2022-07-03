"""
Parse an assembly file
"""
from itertools import chain, permutations, product
from os import getenv
from pathlib import Path
from sly import Parser
from ic4.assembly.srcfile import ExecutableHeader, ObjectsHeader, SourceFile

from ic4.string_utilities import unescape_string_const


from .expressions import Constant, Divide, Multiply, Reference, Subtract, Sum
from .commands import (
    CALL,
    DEC,
    INC,
    INTS,
    JMP,
    LOAD,
    MOV,
    POP,
    PUSH,
    RET,
    STORE,
    ZEROS,
    Directive,
    DirectiveCode,
    Instruction,
    Label,
    OpCode,
    Param,
    ParamMode,
)
from .lexer import ICAssLexer

Path(getenv("LOG_DIR")).mkdir(parents=True, exist_ok=True)  # creating log dir


class ICassSyntaxError(ValueError):
    pass


class ICAssParser(Parser):
    tokens = ICAssLexer.tokens

    debugfile = Path(getenv("LOG_DIR")) / "parser.out"

    precedence = (
        ("left", PLUS, MINUS),
        ("left", TIMES, DIVIDE),
        ("right", UMINUS),  # Unary minus operator
    )

    @_("[ lline_end ] header { command lline_end }")
    def program(self, p):
        return SourceFile(p.header, tuple(chain(*p.command)))

    # Executable header

    @_("EXECUTABLE VERSION lline_end")
    def header(self, p):
        return ExecutableHeader(p.VERSION)

    # Objects header
    @_("OBJECTS VERSION lline_end objs_specs")
    def header(self, p):
        return ObjectsHeader(p.VERSION, **p.objs_specs)

    # producing rules for all possible combinations
    for extern, export, entry in product([True, False], repeat=3):

        @_(
            *(
                " ".join(perm)
                for perm in permutations(
                    comp
                    for p, comp in (
                        (extern, "extern_objs lline_end"),
                        (export, "export_objs lline_end"),
                        (entry, "entry_obj lline_end"),
                    )
                    if p
                )
            )
        )
        def objs_specs(self, p):
            specs = {}
            specs["extern"] = (
                p.extern_objs if hasattr(p, "extern_objs") else frozenset()
            )
            specs["export"] = (
                p.export_objs if hasattr(p, "export_objs") else frozenset()
            )
            specs["entry"] = p.entry_obj if hasattr(p, "entry_obj") else None
            return specs

    @_("EXTERN { IDENTIFIER }")
    def extern_objs(self, p):
        return frozenset(p.IDENTIFIER)

    @_("EXPORT { IDENTIFIER }")
    def export_objs(self, p):
        return frozenset(p.IDENTIFIER)

    @_("ENTRY IDENTIFIER")
    def entry_obj(self, p):
        return p.IDENTIFIER

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
        return INTS(tuple(p.expr))

    @_("INTS STRING")
    def directive(self, p):
        return INTS(p.STRING)

    @_("ZEROS expr")
    def directive(self, p):
        return ZEROS(p.expr)

    @_("INC param")
    def directive(self, p):
        return INC(p.param)

    @_("DEC param")
    def directive(self, p):
        return DEC(p.param)

    @_("MOV param [ COMMA ] param [ COMMA ] [ expr ]")
    def directive(self, p):
        return MOV(p.param0, p.param1, p.expr or Constant(1))

    @_("LOAD param [ COMMA ] param")
    def directive(self, p):
        return LOAD(p.param0, p.param1)

    @_("STORE param [ COMMA ] param")
    def directive(self, p):
        return STORE(p.param0, p.param1)

    @_("JMP param")
    def directive(self, p):
        return JMP(p.param)

    @_("PUSH [ param ] [ COMMA ] [ expr ]")
    def directive(self, p):
        return PUSH(p.param, p.expr or Constant(1))

    @_("POP [ param ] [ COMMA ] [ expr ]")
    def directive(self, p):
        return POP(p.param, p.expr or Constant(1))

    @_("CALL param")
    def directive(self, p):
        return CALL(p.param)

    @_("RET")
    def directive(self, p):
        return RET()

    # --- parameters ---

    @_("param_mode expr")
    def param(self, p):
        return Param(p.param_mode, p.expr)

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
        return Constant(p.NUMBER)

    @_("IDENTIFIER")
    def expr(self, p):
        return Reference(p.IDENTIFIER)

    @_("LPAREN expr RPAREN")
    def expr(self, p):
        return p.expr

    def error(self, token):
        """Error handling routine"""
        if token:
            lineno = getattr(token, "lineno", 0)
            if lineno:
                raise ICassSyntaxError(
                    f"sly: Syntax error at line {lineno}, token={token.type}\n"
                )
            else:
                raise ICassSyntaxError(f"sly: Syntax error, token={token.type}")
        else:
            raise ICassSyntaxError("sly: Parse error in input. EOF\n")
