#ifndef X_MACROS_H_
#define X_MACROS_H_

#define PUNCT(X)		\
	X(OPEN_BRACKET, "[")	\
	X(CLOSE_BRACKET, "]")	\
	X(OPEN_PAREN, "(")	\
	X(CLOSE_PAREN, ")")	\
	X(OPEN_BRACE, "{")	\
	X(CLOSE_BRACE, "}")	\
	X(DOT, ".")		\
	X(ARROW, "->")		\
	X(INCREMENT, "++")	\
	X(DECREMENT, "--")	\
	X(AND, "&")		\
	X(MUL, "*")		\
	X(ADD, "+")		\
	X(SUB, "-")		\
	X(NOT, "!")		\
	X(NOT_EQUAL, "!=")	\
	X(DIV, "/")		\
	X(MOD, "%")		\
	X(LEFT_SHIFT, "<<")	\
	X(RIGHT_SHIFT, ">>")	\
	X(LESS, "<")		\
	X(LESS_EQUAL, "<=")	\
	X(GREATER, ">")		\
	X(GREATER_EQUAL, ">=")	\
	X(EQUAL, "==")		\
	X(ASSIGN, "=")		\
	X(XOR, "^")		\
	X(OR, "|")		\
	X(LOGICAL_AND, "&&")	\
	X(LOGICAL_OR, "||")	\
	X(QUESTION, "?")	\
	X(COLON, ":")		\
	X(SEMICOLON, ";")	\
	X(ELLIPSIS, "...")	\
	X(COMMA, ",")		\
	X(HASH, "#")		\
	X(DOUBLE_HASH, "##")	\

#define KEYWORD(X)		\
	X(IF, "if")		\
	X(ELSE, "else")		\
	X(FOR, "for")		\
	X(DO, "do")		\
	X(WHILE, "while")	\
	X(GOTO, "goto")		\
	X(CASE, "case")		\
	X(DEFAULT, "default")	\
	X(BREAK, "break")	\
	X(CONTINUE, "continue")	\
	X(RETURN, "return")	\
	X(SIZEOF, "sizeof")	\
	X(INLINE, "inline")	\
	X(REGISTER, "register")	\
	X(RESTRICT, "restrict")	\
	X(VOLATILE, "volatile")	\
	X(EXTERN, "extern")	\
	X(UNSIGNED, "unsigned")	\
	X(STATIC, "static")	\
	X(CONST, "const")	\
	X(SWITCH, "switch")	\
	X(TYPEDEF, "typedef")	\
	X(ENUM, "enum")		\
	X(UNION, "union")	\
	X(STRUCT, "struct")	\

#endif
