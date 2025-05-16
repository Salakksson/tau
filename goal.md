# tau will be c with the following features:
﻿
## basic
- modifiers such as `*`, `&` and `[]` bind to the left instead of the right
- `[]` modifier being a slice instead of raw pointer
- a modifier called `ptr` which acts as a smart pointer without <> syntax (`int ptr` = `unique_ptr<int>`)
- another modifier called `arr` which acts as a dynamic array
- structs could have constructors, methods, and maybe some operator overloading
- `long`, `short`, `signed` and`unsigned` doesnt exist and is replaced with `u8`, `i32`, `u256`... and `uint` and `int` which would be guaranteed 32 bits
- `defer`
- enums could be set to a custom size such as `u8`
- `<stdbool.h>` not existing and being the default
- `#use` directive which declares everything which isnt `static` in a source file to eliminate the need for headers
- `if condition: do_something();` and `if condition { do_something(); }`
- `for int i = 0; i < foo; i++ { do_something }`
- `switch` for types other than enums (strings or anything with == operator) and variables
- `switch` break by default, `{case foo: do_something(); fallthrough; case bar: ...}`
- scripting, errors analysed before running to avoid interpreter bullshit
- #!/bin/tauc -- simple af in lexer

## more scary
- better variadic argument parsing (difficult to create a good spec though)
- `any` type which allows the type to change after (`any foo = 5; foo = "5"`)
- `any _` which is predefined and will be used as temporary variable and to store return values of functions
- format strings `f"foo = {foo}\nbar = {bar}"` which can print structs
- `for foo in bar` where bar has a `bar_element? operator [](size_t i)` or iterator bullshit ?maybe but might be anal
- iterate over struct elements such as `for element in foo.*` ?maybe
- wierd macros such as `expression #foo(identifier x) { return ((x.foo)); }`
- `foo = switch (bar) {case baz: 2};`

## TODO: research qbe
