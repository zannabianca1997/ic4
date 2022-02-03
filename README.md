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
- [ ] Miscellanea
    - [ ] Bookmark struct
    - [ ] Error reporting

## Design

Every step is made by a different translation unit, offering a stream-like interface, like the one below:

    /* Unit example */

    // contain an element of the stream
    struct result_s {
        ...
    };

    // delete an element of the stream when it's not used anymore
    void result_free(struct result_s *);

    // keep every value needed for stream working
    typedef ... result_stream_t;

    // open a new stream
    // return NULL on errors
    result_stream_t *result_stream_open(some_stream_identifier);

    // obtain the next stream object
    // return NULL if stream is ended
    struct result_s * result_stream_get(result_stream_t*);

    // return the object to the stream, will be returned by next get
    // guarantee maximum of 1 return, after beaviour is undefinite
    void result_stream_unget(result_stream_t*, struct result_s *);
    
    // close the stream
    void result_stream_close(result_stream_t*);

First translation unit will take a filename, while last will emit integers.