#include "msg.h"
#include "lex.h"
#include "ast.h"
#include <stdio.h>

int main(int argc, char** argv)
{	
	// char* name = argv[0];
	for (int i = 1; i < argc; i++)
	{
		msg(MSG_INFO, argv[i]);
	}

	lexer lex = lexer_create("test/lexer.tau");
	token tok = {0};
	while ((tok = lexer_get_token(&lex)).type != T_EOF)
	{
		switch(tok.type)
		{
			case T_ERR:
				msg(MSG_INFO, "err: %s", tok.err);
				break;
			case T_ID:
				msg(MSG_INFO, "id: %s", tok.str);
				break;
			case T_KEYWORD:
				msg(MSG_INFO, "keyword: %s", view_keyword(tok.e));
				break;
			case T_PUNCT:
				msg(MSG_INFO, "punct: %s", view_punct(tok.e));
				break;
			case T_NLITERAL:
				msg(MSG_INFO, "nliteral: %i", tok.numeric);
				break;
			case T_SLITERAL:
				msg(MSG_INFO, "sliteral: \"%s\"", tok.str);
				break;
			case T_CLITERAL:
				msg(MSG_INFO, "cliteral: '%c'", tok.numeric);
				break;
				default:
				fatal("invalid token");
		}
		token_destroy(&tok);
	}
	lexer_destroy(&lex);
}
