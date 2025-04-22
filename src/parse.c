#include "parse.h"


parser parser_create()
{
	parser p = {0};
	da_construct(p.ast, 20);

	return p;
}


void parser_collect_node(parser* p, lexer* lex)
{
	
}
