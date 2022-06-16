# The Assembler

## The Assembly Language

### Instruction Set

Loosely following the proposed [EBNF](https://en.wikipedia.org/wiki/Extended_Backus%E2%80%93Naur_form) on the [Esolang Wiki](https://esolangs.org/wiki/Intcode), the grammar given to [TatSu](https://github.com/neogeny/TatSu) is:

```EBNF
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

instruction = instr_ADD | instr_MUL | instr_IN | instr_OUT | instr_JNZ 
            | instr_JZ | instr_SLT | instr_SEQ | instr_INCB | instr_HALT ;

instr_ADD  = op:'ADD'  ~  params+:param ~ params+:param ~ params+:param ~ ;
instr_MUL  = op:'MUL'  ~  params+:param ~ params+:param ~ params+:param ~ ;
instr_IN   = op:'IN'   ~  params+:param ~ ;
instr_OUT  = op:'OUT'  ~  params+:param ~ ;
instr_JNZ  = op:'JNZ'  ~  params+:param ~ params+:param ~ ;
instr_JZ   = op:'JZ'   ~  params+:param ~ params+:param ~ ;
instr_SLT  = op:'SLT'  ~  params+:param ~ params+:param ~ params+:param ~ ;
instr_SEQ  = op:'SEQ'  ~  params+:param ~ params+:param ~ params+:param ~ ;
instr_INCB = op:'INCB' ~  params+:param ~ ;
instr_HALT = op:'HALT' ~  ;

param = mode:([ '#' | '@' ]) value:expr ;

expr    = @:addexpr | @:term ;
term    = @:multexpr | @:factor ;
factor  = number | identifier | '('  @:expr  ')' ;

addexpr = left:expr op:('+' | '-') right:term ;
multexpr = left:term op:('*' | '/') right:factor ;

@name
identifier = /[a-zA-Z_][a-zA-Z0-9_]*/ ;

number = /\d+/ ;
```


