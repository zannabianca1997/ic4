# IC4: IntCode C CrossCompiler

## Overview

This is a cross compiler from a dialect of C to [Intcode](https://esolangs.org/wiki/Intcode). 
The idea is that, being Intcode more akin to an instruction set than a language, it can be a good exercise in compilers design.

## Objectives

Objective is to translate the biggest subset of C I can manage.
This is a sintetic roadmap:

- [ ] Preprocessor
    - [ ] Raw line detection
    - [ ] Logical line merging
    - [ ] Tokenization of lines
    - [ ] Preprocessor language
        - [ ] Conditionals
        - [ ] Line control
        - [ ] `#include` directives
        - [ ] Macro substitution
- [ ] Compiler
- [ ] Linker