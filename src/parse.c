#include "parse.h"
#include "lex.h"
#include <string.h>

static ast init_ast()
{
	ast tree;
	da_construct(tree.globals, 10);
	da_construct(tree.functions, 10);
	return tree;
}

static bool tok_is_type(token tok)
{
	// TODO: implement types other than int
	if (tok.type != T_KEYWORD) return false;
	return !strcmp(tok.str, "int");
}

static syntax_node get_type_and_id(ast* tree, lexer* lex)
{
	syntax_node type_and_id = {0};
	bool got_type = false;
	bool got_id = false;

	while (true)
	{
		if (got_type && got_id) break;
		token tok = lexer_get_token(lex);
		if (tok_is_type(tok))
		{
			if (got_type)
			{
				err(tok.loc, "two types");
				break;
			}
			type_and_id.var.type = (type_tree){.kind = TYPE_INTEGER, .width = 32};
			continue;
		}
		if (tok.type == T_ID)
		{
			// assume its not redefined, TODO: check this
			if (got_id)
			{
				err(tok.loc, "two ids");
				break;
			}
			type_and_id.var.name = tok.str;
			continue;
		}
		// TODO: add qualispec parsing
		err(tok.loc, "qualispecs unsupported");
	}
	token tok = lexer_get_token(lex);
	// assume assignement, semi or brace
	if (tok.type != T_PUNCT)
	{
		err(tok.loc, "fuck knows");
		return type_and_id;
	}
	switch (tok.e)
	{
	case P_OPEN_PAREN:
		type_and_id.kind = NODE_FUNCTION;
		return type_and_id;
	case P_SEMICOLON:
		type_and_id.kind = NODE_STATEMENT;
		return type_and_id;
	case P_ASSIGN: // TODO: add +=, -=, *=, ^= ...
		type_and_id.kind = NODE_LVALUE;
		return type_and_id;
	default:
		err(tok.loc, "expected = ; or (");
		return type_and_id;
	}
}

static syntax_node get_expression(lexer* lex)
{
	// todo: parse shit more complex than a literal
	syntax_node expr = {0};
	token tok = lexer_get_token(lex);
	switch (tok.type)
	{
	case T_NLITERAL:
	case T_SLITERAL:
	case T_CLITERAL:
		expr.kind = NODE_LITERAL;
		expr.literal = tok;
	default:
		expr.kind = NODE_ERROR;
		err(tok.loc, "expected literal");
	}
}

static syntax_node get_function(ast* tree, lexer* lex)
{
	todo("idfk");
}

static void get_global_node(ast* tree, lexer* lex)
{
	// a global node can be either a function or variable declaration
	// both require a type, id, then either = ; or a brace
	syntax_node node = get_type_and_id(tree, lex);
	if (node.kind == NODE_STATEMENT)
	{
		da_append(tree->globals, node.var);
		return;
	}
	if (node.kind == NODE_LVALUE)
	{
		syntax_node value = get_expression(lex);
		todo("read lvalues");
	}
	if (node.kind == NODE_FUNCTION)
	{
		todo("go fuck yourself, a function? u fr?");
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
