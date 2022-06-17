# End-to-end testing

These are complete program files. Each one should come paired with a same name json that specify a series of inputs and expected outputs. If missing input, it will be considered `[]`. If input or output are strings, they will be interpreted as a list of their unicode codepoint-

```json
[
    {
        "name":"normal test",
        "descr":"this is a normal test",
        "input":[1,2,3,25,0],
        "output":[2,3]
    },
    {
        "name":"string test",
        "descr":"this is a string test",
        "input":"this is the input to give to the program",
        "output":"this is the expected output."
    }
]
```