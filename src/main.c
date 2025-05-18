#include "diag.h"
#include "lex.h"
#include "parse.h"

int main(int argc, char** argv)
{
	// char* name = argv[0];
	for (int i = 1; i < argc; i++)
	{
		info(argv[i]);
	}
	lexer lex = lexer_create("test/lexer.tau");
	token tok = {0};
	while ((tok = lexer_get_token(&lex)).type != T_EOF)
	{
		switch(tok.type)
		{
			case T_ID:
				info("id: %s", tok.str);
				break;
			case T_KEYWORD:
				info("keyword: %s", view_keyword(tok.e));
				break;
			case T_PUNCT:
				info("punct: %s", view_punct(tok.e));
				break;
			case T_NLITERAL:
				info("nliteral: %i", tok.numeric);
				break;
			case T_SLITERAL:
				info("sliteral: \"%s\"", tok.str);
				break;
			case T_CLITERAL:
				info("cliteral: '%c'", tok.numeric);
				break;
			default:
				fatal("invalid token");
		}
		token_destroy(&tok);
	}
	lexer_destroy(&lex);
}
