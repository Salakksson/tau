#ifndef LEX_H_
#define LEX_H_

#include <stdlib.h>
#include <stdbool.h>

typedef struct var var;
typedef struct pmd pmd;

typedef enum var_kind
{
	VAR_NULL = 0,
	VAR_ATOM,
	VAR_LIST,
	VAR_BLOCK,
} var_kind;

typedef enum block_kind
{
	BLOCK_PMD,
	BLOCK_SHELL,
	BLOCK_C,
} block_kind;

typedef enum atom_kind
{
	ATOM_STRING,
	ATOM_NUMBER,
	ATOM_VAR,
} atom_kind;

typedef struct var_atom
{
	atom_kind kind;
	union {
		char* str;
		ssize_t value;
	};

} var_atom;

typedef struct var_list
{
	var* vars;
	size_t vars_count;
	size_t vars_size;
} var_list;

typedef struct cmd
{
	char* name;
	var_list args;
} cmd;

typedef var (*c_func)(pmd*, var_list);
// var foo(var_list, bool)

typedef struct var_block
{
	block_kind kind;
	union {
		struct {
			cmd* cmds;
			size_t cmds_count;
			size_t cmds_size;
		};
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

typedef struct lexer
{
	const char* source;

	char* buffer;
	size_t ptr;
	size_t lineno;

	bool err;
} lexer;

lexer lexer_create(const char* source);

cmd lexer_get_cmd(lexer* lex);
cmd lexer_peek_cmd(lexer* lex);

#endif
