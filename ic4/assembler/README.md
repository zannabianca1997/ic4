# The Assembler

## The Assembly Language

Extension for assembly files is `.ica`, for compiled files `.int`.

### Instruction Set


There are three basic units in the language:
- **Labels:** written as `name:` they mark the next position as `name`.
- **Directives:** instruction for the assembler.
- **Instructions:** IntCode instructions.

## Instructions

See [the IntCode specifications](https://esolangs.org/wiki/Intcode). Parameter modes are specified by a prefix: `#` for immediate mode, `@` for relative, and none for absolute.


## Directives

### INTS
Takes a variadic number of parameters, and copies them verbatim. Parameters can be any expression.

### ZEROS
Takes a single expression, that must be resolved with labels preceding it. Add that number of zeros to the program.

### INC
Takes a param. Equivalent to `ADD {param} #1 {param}`, it INCrease its value
### DEC
Takes a param. Equivalent to `ADD {param} #-1 {param}`, it DECrease its value
