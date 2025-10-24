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
