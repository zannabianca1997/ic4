"""
    Parse a file into an AST
"""
from pathlib import Path
from typing import Iterator

from .instructions import Instruction
from tatsu import compile
from tatsu.model import ModelBuilderSemantics

Parser = compile(r"""
@@grammar::IC4ASS
@@parseinfo::False

@@keyword :: DATA
@@keyword :: ADD MUL IN OUT JNZ JZ SLT SEQ INCB HALT

@@eol_comments :: /;.*?$/

file::File = { lines+:line } $ ;

line::Line = labels:{ label ~ } content:( directive | instruction ) ;

label = @:identifier ':' ;

directive::Directive = name:dirname ~ { params+:expr ~ [',']} ;
dirname::str = 'DATA' ;

instruction::Instruction = opcode:op ~ { params+:param ~ [',']} ;

op::str = 'ADD' | 'MUL' | 'IN' | 'OUT' | 'JNZ' | 'JZ' 
      |   'SLT' | 'SEQ' | 'INCB' | 'HALT' ;

param::Parameter = mode:([ '#' | '@' ]) value:expr ;

expr    = @:addexpr | @:term ;
term    = @:multexpr | @:factor ;
factor  = number | identifier | '('  @:expr  ')' ;

addexpr::AddExpr = left:expr op:('+' | '-') right:term ;
multexpr::MulExpr = left:term op:('*' | '/') right:factor ;


@name
identifier::str = /[a-zA-Z_][a-zA-Z0-9_]*/ ;

number::int = /\d+/ ;
""",
                 semantics=ModelBuilderSemantics())


def parse_file(file: Path):
    with open(file) as inp:
        return Parser.parse(file.read(), start="file")
