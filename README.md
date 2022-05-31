# IC4: IntCode C CrossCompiler

> :hammer: **WARNING:** This repository is a work in progress. Software is still not functional. See [Objectives](##Objectives) for a roadmap. :wrench:

## Overview

This is a cross compiler from a dialect of C to [Intcode](https://esolangs.org/wiki/Intcode). 
The idea is that, being Intcode more akin to an instruction set than a language, it can be a good exercise in compilers design.
This is nothing more than an exercise: no particular standard is adhered to, neither any particular feature will be implemented.

## Objectives

Objective is to translate the biggest subset of C I can manage.
This is a sintetic roadmap:

- [ ] Preprocessor
    - [ ] Raw line detection
    - [ ] Logical line merging
    - [ ] Tokenization of line
    - [ ] Parsing directives
    - [ ] File managing
    - [ ] Preprocessor language
        - [ ] Conditionals
        - [ ] Line control
        - [ ] `#include` directives
        - [ ] Macro substitution
- [ ] Compiler
- [ ] Linker
- [ ] Assembler
- [ ] Miscellanea
    - [ ] Context tracking
    - [ ] Error reporting
    - [ ] Bookmark struct
    - [ ] Test discovery and running

## Used C dialect

To adapt to the end instruction limitations, the original language will deviate from standard C.

The most noticeable difference from standard C is that scalar type will be a wreak: only `signed int` is implemented, and is the natural size of the machine (`sizeof(int) == 1`). Other `signed` integers may, or may not be translated as `int`. `unsigned` variation should be inplemented with adapt code for the wrap around (C stantdard require `unsigned` wraparound, while `signed` owerflow is UB).

`float`s are for now not implemented, nor planning to be.

For the aritmetic `+`, `-` and `*` will translate directly to code. `/` and `%` are planned to be translated to calls to builtin functions.  
There is no plan to implement bitshift or bitwise operators, as they would be more slow than useful.

## Running

For now (and the foreseeable future) only GCC is tested for compilation.


## Design

Every step is made by a different translation unit, offering a stream-like interface, like the one below:

```C
/* Unit example */

// contain an element of the stream
struct result_s {
    ...
};

// delete an element of the stream when it's not used anymore
void result_free(struct result_s *);

// keep every value needed for stream working
typedef ... resultstream_t;

// open a new stream
// return NULL on errors
result_stream_t *resultstream_open(some_stream_identifier);

// obtain the next stream object
// return NULL if stream is ended
struct result_s * resultstream_get(result_stream_t*);

// return the object to the stream, will be returned by next get
// guarantee maximum of 1 return, after beaviour is undefinite
void resultstream_unget(result_stream_t*, struct result_s *);

// close the stream
void resultstream_close(result_stream_t*);
```

First translation unit will take a filename, while last will emit integers.

## Compile flags

These flags are used to compile different capacity in the program


## Developers

For now, everything is by me, zannabianca1997 <zannabianca199712@gmail.com>.
