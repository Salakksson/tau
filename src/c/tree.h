#ifndef TREE_H_
#define TREE_H_

#include "lex.h"

#include <stdlib.h>

typedef struct var var;
typedef struct pmd pmd;

typedef enum {
	VAR_NULL = 0,
	VAR_ATOM,
	VAR_LIST,
	VAR_BLOCK,
} var_kind;

typedef enum {
	BLOCK_PMD,
	BLOCK_SHELL,
	BLOCK_C,
} block_kind;

typedef enum {
	ATOM_STRING,
	ATOM_NUMBER,
	ATOM_VAR,
} atom_kind;

typedef struct
{
	atom_kind kind;
	union {
		char* str;
		ssize_t value;
	};

} var_atom;

typedef struct
{
	var* arr;
	size_t len;
	size_t cap;
} var_list;

typedef struct
{
	char* cmd;
	var_list args;
} statement;

typedef var (*c_func)(pmd*, var_list);

typedef struct
{
	block_kind kind;
	union {
		struct {
			statement* sts_arr;
			size_t sts_len;
			size_t sts_cap;
			var_list args;
		} pmd;
		char* shell;
		c_func c;
	};

} var_block;

typedef struct var
{
	var_kind kind;
	union {
		var_atom atom;
		var_list list;
		var_block block;
	};
} var;

typedef struct
{
	lexer* lex;
	struct {
		token* arr;
		size_t len;
		size_t cap;
	} tokens;
	size_t current_token;

	bool err;
} parser;

char* var_to_str(pmd* p, var v);
char* var_to_debug(var v);
void var_list_append(var_list* list, var v);

var create_tree(const char* source);

#endif
