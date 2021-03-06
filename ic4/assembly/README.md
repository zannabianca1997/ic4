# The Assembler

## The Assembly Language

Extension for assembly files is `.ica`, for compiled files `.int`. See the [end-to-end tests](../../tests/assembler/end_to_end)

### Grammar

There are three basic units in the language:
- **Header:** specify what type of file is this
- **Labels:** written as `name:` they mark the next position as `name`.
- **Directives:** instruction for the assembler, ended by a newline.
- **Instructions:** IntCode instructions, ended by a newline.

## Header

Header specify if this is a *object* file, that can be linked with other, or if it's an *executable*, that will produce a IntCode program. It must come before any instruction or directive.

### Object files
Header is:
```
OBJECTS version
EXPORT a b c ... 
EXTERN d e f ... 
ENTRY  main
```
The only version that is now supported is 0.1.
The three subsequential lines are optionals, but must mantain the order:
- `EXPORT` list the labels that must be accessible by other object files.
- `EXTERN` instead list the label that are finded into other files. They must not be defined into the file
- `ENTRY` signal that that label is the program starting point.

### Executable files
Header is:
```
EXECUTABLE version
```
The only version that is now supported is 0.1. If header is missing, this is the default (`EXECUTABLE 1.0`)


## Labels

They can appear on a line by themselves, or before any directive or instruction. They are composed like `identifier :`. They mark the *next cell* with `identifier`, that's freely usable in expression (even ones before, with the exception of variable lenght directive arguments).

## Instructions

See [the IntCode specifications](https://esolangs.org/wiki/Intcode). Parameter modes are specified by a prefix: `#` for immediate mode, `@` for relative, and none for absolute.

## Directives

### INTS
Takes a variadic number of parameters, and copies them verbatim. Parameters can be any expression.

### ZEROS
Takes a single expression, that must be resolved with labels preceding it. Add that number of zeros to the program.

### INC
Takes a param. Equivalent to `ADD {a} #1 {a}`, it INCrease its value
### DEC
Takes a param. Equivalent to `ADD {a} #-1 {a}`, it DECrease its value

## JMP
Unconditional jump to its only parameter

### MOV
Takes two param or two param and an optional expression, that must be resolved with labels preceding it.
Equivalent to `ADD {a} #0 {b}`, moving the value of the first param to the second.
If the expression is present, it's equivalent to 
```
ADD {a} #0 {b}
ADD {a}+1 #0 {b}+1
...
ADD {a}+n #0 {b}+n
```
Note that `a` and `b` mantain the original mode. If `a` is immediate the memory will instead be filled with the same value.
The move is always made from the first to the last. It's then safe to move overlapping memory only towards 0.

### LOAD
Takes two params. The value stored in the **cell** at the position given by the first param value is moved into the second param. `LOAD {a} {b}` is equivalent to the pseudocode `b = mem[a]`.
For now `LOAD` is implemented with minimal self modifing code. Without some guarantees on stack presence and structure (control on the relative pointer) this is the best INTCODE permit. Assembled `LOAD` code is not position indipendent, even if its parameter are.

### STORE
Takes two params. The value of the first param is moved in the **cell** at the position given by the second param value. `STORE {a} {b}` is equivalent to the pseudocode `mem[b] = a`
For now `STORE` is implemented with minimal self modifing code. Without some guarantees on stack presence and structure (control on the relative pointer) this is the best INTCODE permit. Assembled `STORE` code is not position indipendent, even if its parameter are.

### PUSH
Takes an optional param and an optional expression. Uses relative mode to manage a stack: `PUSH {a} [size]` is equivalent to
```
MOV {a} @0 [size]
INCB [size]
```
As in `MOV` `size` is assumed 1 if not present. If `a` is absent, unspecified data will be loaded. After the `PUSH` the pushed values starts then at `@-size` up to `@0`.
Also like `MOV` if `a` is in immediate mode the space will be filled with the given value.
Take care that `PUSH` do not create the stack: an appropriate `RB` must be setted beforehand

### POP
Takes an optional param and an optional expression. Uses relative mode to manage a stack: `POP {a} [size]` is equivalent to
```
INCB -[size]
MOV @0 {a} [size]
```
As in `PUSH` `size` is assumed 1 if not present. If `a` is absent, the data will be discarded. To specify a size with `a` absent one can use the optional comma separator.
Take care that `POP` do not create the stack: an appropriate `RB` must be setted beforehand

## CALL
Takes a parameter. `PUSH` the instruction pointer of the next instruction on the stack. Then jump to the parameter. `CALL {a}` is equivalent to 
```
PUSH __ret
JMP {a}
__ret:
```

## RET
`POP` a value from the stack and jumps to it
Equivalent to 
```
INCB -1
JMP @0
```

## Expression

Expression are composed by the four operations, brakets, number and identifiers. All instructions and directives accept optional commas to separe expressions.

## Numbers
All numbers are integers, and can be spexified in four ways:
- **Decimal:** `10`, `34`, ...
- **Hexadecimal:** `0x2a`, `0xFF`, ...
- **Char constant:** `'a'`, `'\n'`, `'\0'`, `'\xAA'`, ...

## Identifiers
Identifiers in *objects* files follow the rules of C identifiers. In *executables* files the character `$` is also available (used to add file information).