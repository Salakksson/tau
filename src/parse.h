#ifndef PARSE_H_
#define PARSE_H_

#include "da.h"
#include "lex.h"
#include "types.h"

typedef struct
{
	type_tree type;
	char* name;
} variable;

typedef enum
{
	NODE_ERROR = 0,
	NODE_FUNCTION,
	NODE_STATEMENT,
	NODE_EXPRESSION,
	NODE_LVALUE,
	NODE_VARIABLE,
	NODE_LITERAL,
	NODE_SCOPE,
} node_kind;

typedef struct syntax_node
{
	node_kind kind;
	union {
		token op;
		token literal;
		variable var;
		DA(variable) vars;
	};
	DA(struct syntax_node) nodes;
} syntax_node;

typedef struct
{
	DA(variable) globals;
	DA(syntax_node) functions;
} ast;

ast parse(lexer* lex);

#endif
