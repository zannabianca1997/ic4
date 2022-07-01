# IC4: IntCode C CrossCompiler

> :hammer: **WARNING:** This repository is a work in progress. Software is still not functional. See [Objectives](##Objectives) for a roadmap. :wrench:

## Overview

This is a cross compiler from a dialect of C to [Intcode](https://esolangs.org/wiki/Intcode). 
The idea is that, being Intcode more akin to an instruction set than a language, it can be a good exercise in compilers design.
This is nothing more than an exercise: no particular standard is adhered to, neither any particular feature will be implemented.

## Objectives

Objective is to translate the biggest subset of C I can manage.
This is a sintetic roadmap:

- [ ] Compiler
- [X] Assembly parser
  - [X] Lexer
  - [X] Header
  - [X] Instructions
  - [X] Labels
  - [X] Directives
    - [X] INTS
    - [X] STRING
    - [X] ZEROS
    - [X] JMP
    - [X] INC and DEC
    - [X] MOV
    - [X] LOAD and STORE
    - [X] PUSH and POP
    - [X] CALL and RET
- [ ] Linker
  - [ ] Reference consistency check
  - [ ] Label renaming
  - [ ] Entry code generation
- [X] Assembler
  - [X] Code generation
- [X] Intcode machine

## Used C dialect

To adapt to the end instruction limitations, the original language will deviate from standard C.

The most noticeable difference from standard C is that scalar type will be a wreak: only `signed int` is implemented, and is the natural size of the machine (`sizeof(int) == 1`). Other `signed` integers may, or may not be translated as `int`. `unsigned` variation should be inplemented with adapt code for the wrap around (C stantdard require `unsigned` wraparound, while `signed` owerflow is UB).

`float`s are for now not implemented, nor planning to be.

For the aritmetic `+`, `-` and `*` will translate directly to code. `/` and `%` are planned to be translated to calls to builtin functions.  
There is no plan to implement bitshift or bitwise operators, as they would be more slow than useful.


## Developers

For now, everything is by me, zannabianca1997 <zannabianca199712@gmail.com>.
