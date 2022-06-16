"""
    Parse a file into an AST
"""
from pathlib import Path
from tatsu import compile

from .instructions import Directive, DirectiveCode, Instruction, Label, OpCode, ParamMode
from .expressions import Divide, Multiply, Subtract, Sum


class IC4AssSemantic:
    def number(self, ast):
        return int(ast)

    def label(self, ast):
        return Label(ast, None)

    def instruction(self, ast):
        return Instruction(
            OpCode[ast["op"]],
            tuple(ast["params"]) if ast["params"] is not None else ()
        )

    def param(self, ast):
        mode = ParamMode.MODE0
        if ast["mode"] == '#':
            mode = ParamMode.MODE1
        elif ast["mode"] == '@':
            mode = ParamMode.MODE2
        return mode, ast["value"]

    def directive(self, ast):
        return Directive(
            DirectiveCode[ast["code"]],
            tuple(ast["params"]) if ast["params"] is not None else ()
        )

    def addexpr(self, ast):
        if ast["op"] == "+":
            return Sum(ast['left'], ast['right'])
        return Subtract(ast['left'], ast['right'])

    def multexpr(self, ast):
        if ast["op"] == "*":
            return Multiply(ast['left'], ast['right'])
        return Divide(ast['left'], ast['right'])


GRAMMAR = r"""
@@grammar::IC4ASS
@@parseinfo::False

@@keyword :: INTS
@@keyword :: ADD MUL IN OUT JNZ JZ SLT SEQ INCB HALT

@@eol_comments :: /;.*?$/

file = { command } $ ;

command = ( label | directive | instruction ) ;

label = @:identifier ':' ;

directive = direct_INTS ;
direct_INTS = code:'INTS' ~ '(' { params+: expr [','] ~ } ')' ;

instruction = <instruction_rules_list> ;

<instruction_rules>

param = mode:([ '#' | '@' ]) value:expr ;

expr    = @:addexpr | @:term ;
term    = @:multexpr | @:factor ;
factor  = number | identifier | '('  @:expr  ')' ;

addexpr = left:expr op:('+' | '-') right:term ;
multexpr = left:term op:('*' | '/') right:factor ;

@name
identifier = /[a-zA-Z_][a-zA-Z0-9_]*/ ;

number = /\d+/ ;
""".replace(
    "<instruction_rules_list>",
    " | ".join(f"instr_{opcode.name}" for opcode in OpCode)
).replace(
    "<instruction_rules>",
    "\n".join(
        f"instr_{opcode.name} = op:'{opcode.name}' ~ {' params+:param ~'*opcode.param_number()} ;"
        for opcode in OpCode
    )
)

Parser = compile(GRAMMAR, semantics=IC4AssSemantic())


def parse_file(file: Path):
    with open(file) as inp:
        return Parser.parse(inp.read(), start="file", trace=True, colorize=True)
