#include "parse.h"

#include "msg.h"

parser parser_create()
{
	parser p = {0};
	da_construct(p.ast, 20);

	return p;
}

bool id_is_type(parser* p, char* id)
{
	// todo
	return false;
}

bool id_is_var(parser* p, char* id)
{
	// todo
	return false;
}

bool token_is_function_qualispec(token tok)
{
	if (tok.type != T_KEYWORD) return false;
		
	switch (tok.e)
	{
		case K_INLINE:
		case K_EXTERN:
		case K_STATIC:
			return true;
		default:
			return false;
	}
}

bool token_is_variable_qualispec(token tok)
{
	if (tok.type != T_KEYWORD) return false;
	
	switch (tok.e)
	{
		case K_REGISTER:
		case K_RESTRICT:
		case K_VOLATILE:
		case K_EXTERN:
		case K_STATIC:
		case K_CONST:
			return true;
		default:
			return false;
	}
}

bool token_is_type_declaration(token tok)
{
	if (tok.type != T_KEYWORD) return false;

	switch (tok.e)
	{
		case K_TYPEDEF:
		case K_ENUM:
		case K_UNION:
		case K_STRUCT:
			return true;
		default:
			return false;
	}
}

bool token_is_begin_global(parser* p, token tok)
{
	if (tok.type == T_ID) return id_is_type(p, tok.str) || id_is_var(p, tok.str);
	if (tok.type != T_KEYWORD) return false;

	return  token_is_function_qualispec(tok) ||
		token_is_variable_qualispec(tok) ||
		token_is_type_declaration(tok);
}

void parser_append_error(parser* p, token tok)
{
	fatal("todo: error handling ☺");
}

bool parser_collect_node(parser* p, lexer* lex)
{
	token tok = lexer_peek_token(lex);
	if (tok.type == T_EOF) return false;
	if (tok.type == T_ERR)
	{
		parser_append_error(p, tok);
		lexer_get_token(lex);
		return true;
	}
	if (!token_is_begin_global(p, tok))
	{
		// TODO: Handle error by pushing error node
		err("invalid token");
		return false; // temporary
	}
	
	return true;
}

