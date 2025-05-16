#include "parse.h"
#include "lex.h"



static ast init_ast()
{
	ast tree;
	return tree;
}

static bool tok_is_type(token tok)
{

	return true;
}

static void get_global_node(ast* tree, lexer* lex)
{
	token tok = lexer_get_token(lex);
	if (tok_is_type(tok))
	{
		;
	}
}

ast parse(lexer* lex)
{
	ast tree = init_ast();
	while (true)
	{
		token tok = lexer_peek_token(lex);
		if (tok.type == T_EOF) return tree;

		get_global_node(&tree, lex);
	}
}
