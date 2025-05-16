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

typedef struct
{
	DA(variable) globals;
} ast;

ast parse(lexer* lex);


#endif
