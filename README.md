# Tau - If C was simpler

## Disclaimer

Currently many features described in this document are unimplented,
many of the features here are planned and not implemented fully.
This language is nowhere near complete and even the basic examples
are not implemented, currently im in the process of implementing a
lot of control flow which is why most of the examples wont run.

This readme document is also unfinished and doesnt contain many
features of the language, these will be added to this document soon.

This repo is also currently untidy from when this was strictly a pamde
config file parser, im working on a rewrite of the codebase.

## Description

Tau is a programming language i first made for pamde, but decided to
expand it into a full programming language, its designed to be a mix
of C (function and variable declarations) and lisp (prefix notation
for calling functions, and some use of s-expressions).

The main idea behind the language is to maintain both readability
and writability. The idea is to strip unnecessary punctuators, be
easy to parse, faster to write and have modern features such as easy
error handling, debugging, use of data structures, etc.

The language is primarily based off only a few objects/syntax nodes:
### expressions
* function calls: `fn arg1 arg2`
* infix expressions: `a + 2`
* assignment: `int a = <expr>;`
### objects
* blocks: `{<expr>; <expr>}`
* parens: `(<expr>; <expr>)`
* lists: `[<atom> <atom> <atom>]`
### functions
* declare: `int add(int a, int b) <block>`
* references: `int(int, int) operation = add`

parens are the same as blocks but evaluated before being passed to a function

An example of the syntax is shown below in an implementation of cat:
```
use "io.tau";
use "fs.tau" as fs;

int main(string[] args)
{
	expect (args.count >= 1) "cat: no filed provided";
	for string s in args {
		string contents = fs.read_file s;
		echo contents;
	}
}
```
Notice how the infix expression `args.count >= 1` is in parenthesis?
This is due to the order of operations where math expressions are less
significant than a function call, for example `add 1 2 * 3` is the
equivalent of `add(1, 2) * 3` and not `add(1, 2 * 3)`.
The `=` operator is of the lowest precidence which is why infix expressions
are even allowed unlike for example bash where `$(())` must be used.
The first line could be rewritten as:
```
bool invalid_args = args.count >= 1;
expect invalid_args "cat: no filed provided";
```
This is quite unique (i havent personally seen a language which mixes
infix and prefix expressions but it might exist) so some basic features
can be rethought, for example the `;` can be thought to act like C's
`,` operator, which evaluates all expressions and returns the last expression.

This semicolon can be used in ways such as assigning a variable to a block
```
int a = {0, 1};
int b = if true 2 else 1;
int c = for i in [0, 1, 2, 3] {i};
# equivalent to:
int a = 1;
int b = 2;
int c = 3;
```

## Contributing
This project is open to pull requests but make sure to adhere to the following guidelines:

* Use tab characters for indentation, and up to 3 space characters after the indentation
tabs for allignment

Reason: so that people can choose their own tab width while editing
and allignment is consistent across as many environments as possible

* Lines of code should be no more than 80 characters wide (when viewed with tabs of width 8 characters),
unless shortening them would require breaking strings which are visible to the user
```c
struct s good = (struct s) {
	.a = a,
	.b = b,
	.c = c,
	.d = d
}
const char* bad = // if i grep for "The input provided is not supported" i wont find this
	"Error: The input provide"
	"d is not supported";
```
Reason: Although most screens are wider than 80 characters, long lines of code are still
less readable and may be broken if the screen is split in half (such as when editing two files).
The exception is obviously when a long string is printed to the screen and you may want to
grep for it in the source code to find where it originates from easilly.

* `goto` statements are permitted for non-excessive use (such as error handling, existing nested loops etc).

* The open brace (`{`) character should be on a new line on its own, or on a line consisting of only a closing parenthesis.
The exception being structs/unions
```c
int good(void)
{
	...
}

int bad(void) {
	...
}

int good-long (
	int a,
	int b,
	int c,
	int d
) {
	...
}

int bad-long (
	int a,
	int b,
	int c,
	int d )
{
	...
}

typedef struct {
	int foo;
	struct {
		int bar;
		int baz;
	} good_2;
} good;

typedef struct {
	int foo;
	struct
	{
		int bar;
		int baz;
	} good_2;
} bad;

if (good) do_something();
if (good)
	do_something();
if (good)
{
	do_something();
}

if (bad) { do_something(); }
if (bad) {
	do_something();
}
```
Reason: This isnt as objective as the other reasons but in this style the open and close braces are
on the same level, making it personally easier for me to see the beginning and end of a scope.
The long case is allowed for the reason of shortening lines with definitions of functions with many arguments.
The exception for structs is because the `{}` doesnt define a scope, same with the `()`
of a function declaration which is why the `good-long` example is fine

* Avoid using empty arguments, and prefer `void`
```c
void do_something(); // bad
void do_something(void); // good
```
Reason: In older versions of the C standard, `()` meant the equivalent of `(...)` today, meaning
`do_something("", 29, 7.3f);` would be valid with the bad declaration, this is not as important
but its preferable to specifically mark a function as taking no arguments using `void`.

* Refrain from heavy use of preprocessor macros

Reason: Although the preprocessor is a great C feature and is very useful for simplifying code,
it can also be dangerous as one misplaced `\` could break a long macro, and it prevents the compiler
from giving certain warnings/feedback about the macro and may require `-E` to debug which is not always nice.
Long function-like macros should be used sparingly, and only if using a regular function wouldnt work.

* Use the `-Werror` flag or an equivalent, and enable most warnings

Reason: Often times warnings are ignored, although a lot of the time they are pointless, its still better
to take care of them since at one point or another one of the warnings will be there to alert
you of a bug and if you ignore the warnings and let them pile up, you are less likely to
spot the important warning.

* Use 1 empty line between definitions of functions/structs/long comments

Reason: This separates different parts of the code slightly, and makes it easier
to move functions by cutting/pasting in most cases.
