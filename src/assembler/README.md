# The Assembler

## The Assembly Language

### Instruction Set

Following the proposed [EBNF](https://en.wikipedia.org/wiki/Extended_Backus%E2%80%93Naur_form) on the [Esolang Wiki](https://esolangs.org/wiki/Intcode):

```EBNF
input ::= { line '\n' } ;

line  ::= [ identifier ':' ] [ instr | directive ] ;

directive ::= 'DATA' { expr [','] } ;

instr ::= op { param [','] } ;

op    ::= 'ADD' (* 01 *) | 'MUL' (* 02 *) | 'IN'  (* 03 *) | 'OUT'  (* 04 *) | 'JZ'   (* 05 *)
        | 'JNZ' (* 06 *) | 'SLT' (* 07 *) | 'SEQ' (* 08 *) | 'INCB' (* 09 *) | 'HALT' (* 99 *) ;

param ::= [ '#' (* mode 1 *) | '@' (* mode 2 *) ] expr ;

expr  ::= expr '+' expr | expr '-' expr
        | expr '*' expr | expr '/' expr
        | '+' expr | '-' expr
        | '(' expr ')' | identifier | number ; (* with the usual precedence resolution *)

expr   ::= term   { ("+" | "-") term} ;
term   ::= factor { ("*"|"/") factor} ;
factor ::= number | identifier | "("  expr  ")" ;
```
