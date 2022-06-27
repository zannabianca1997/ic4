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
Takes a param. Equivalent to `ADD {a} #1 {a}`, it INCrease its value
### DEC
Takes a param. Equivalent to `ADD {a} #-1 {a}`, it DECrease its value

### MOV
Takes two param or two param and a expression, that must be resolved with labels preceding it.
Equivalent to `ADD {a} #0 {b}`, moving the value of the first param to the second.
If the expression is present, it's equivalent to 
```
ADD {a} #0 {b}
ADD {a}+1 #0 {b}+1
...
ADD {a}+n #0 {b}+n
```
The move is always made from the first to the last. It's then safe to move overlapping memory only towards 0.

### LOAD
Takes two params. The value stored in the **cell** at the position given by the first param value is moved into the second param. `LOAD {a} {b}` is equivalent to the pseudocode `b = mem[a]`.
For now `LOAD` is implemented with minimal self modifing code. Without some guarantees on stack presence and structure (control on the relative pointer) this is the best INTCODE permit. Assembled `LOAD` code is not position indipendent, even if its parameter are.
### STORE
Takes two params. The value of the first param is moved in the **cell** at the position given by the second param value. `STORE {a} {b}` is equivalent to the pseudocode `mem[b] = a`
For now `STORE` is implemented with minimal self modifing code. Without some guarantees on stack presence and structure (control on the relative pointer) this is the best INTCODE permit. Assembled `STORE` code is not position indipendent, even if its parameter are.