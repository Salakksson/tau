#include "diag.h"
#include "lex.h"
#include "parse.h"

int main(int argc, char** argv)
{
	// char* name = argv[0];
	for (int i = 1; i < argc; i++)
	{
		diag(DIAG_INFO, argv[i]);
	}

	lexer lex = lexer_create("test/lexer.tau");
	token tok = {0};
	while ((tok = lexer_get_token(&lex)).type != T_EOF)
	{
		switch(tok.type)
		{
			case T_ERR:
				diag(DIAG_INFO, "err: %s", tok.err);
				break;
			case T_ID:
				diag(DIAG_INFO, "id: %s", tok.str);
				break;
			case T_KEYWORD:
				diag(DIAG_INFO, "keyword: %s", view_keyword(tok.e));
				break;
			case T_PUNCT:
				diag(DIAG_INFO, "punct: %s", view_punct(tok.e));
				break;
			case T_NLITERAL:
				diag(DIAG_INFO, "nliteral: %i", tok.numeric);
				break;
			case T_SLITERAL:
				diag(DIAG_INFO, "sliteral: \"%s\"", tok.str);
				break;
			case T_CLITERAL:
				diag(DIAG_INFO, "cliteral: '%c'", tok.numeric);
				break;
			default:
				fatal("invalid token");
		}
		token_destroy(&tok);
	}
	lexer_destroy(&lex);
}
