#ifndef PARSE_H_
#define PARSE_H_

#include "da.h"

typedef enum
{
	O_ASSIGN,
} operator_kind;

typedef struct syntax_node syntax_node;
da_declare(syntax_node_list, syntax_node);

typedef struct
{
	syntax_node* left;
	syntax_node* right;
	operator_kind oper;
} expr_binary;

typedef struct 
{
	syntax_node_list statements;
} sn_function;

typedef enum
{
	SN_EXPR,
	SN_FUNCTION,
} sn_type;

struct syntax_node
{
	sn_type type;
	union {
		expr_binary binary;
	};
};

typedef struct 
{
	syntax_node_list nodes;
} ast;

#endif
