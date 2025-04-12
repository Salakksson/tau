#include "lex.h"
#include "msg.h"

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
				msg(MSG_INFO, "err: %i:%i %s", tok.loc.line, tok.loc.col, tok.value.err);
				break;
			case T_ID:
				msg(MSG_INFO, "id: %i:%i %s", tok.loc.line, tok.loc.col, tok.value.str);
				break;
			case T_KEYWORD:
				msg(MSG_INFO, "keyword: %i:%i %s:%i", tok.loc.line, tok.loc.col, view_keyword(tok.value.e), tok.value.e);
				break;
			case T_PUNCT:
				msg(MSG_INFO, "punct: %i:%i %s:%i", tok.loc.line, tok.loc.col, view_punct(tok.value.e), tok.value.e);
				break;
			case T_NLITERAL:
				msg(MSG_INFO, "nliteral: %i:%i %i", tok.loc.line, tok.loc.col, tok.value.numeric);
				break;
			case T_SLITERAL:
				msg(MSG_INFO, "sliteral: %i:%i \"%s\"", tok.loc.line, tok.loc.col, tok.value.str);
				break;
			case T_CLITERAL:
				msg(MSG_INFO, "cliteral: %i:%i '%c'", tok.loc.line, tok.loc.col, tok.value.numeric);
				break;
			default:
				fatal("invalid token");
		}
		token_destroy(&tok);
	}
	lexer_destroy(&lex);
}
