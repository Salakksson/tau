#include "tree.h"
#include "lex.h"

#include "da.h"

static void parser_throw(parser* p, const char* fmt, ...)
{
	p->err = true;
	va_list args;
	va_start(args, fmt);

	location loc = p->tokens.arr[p->current_token].loc;

	fprintf(stderr, "error at %s:%i:%i: ", p->lex->source, loc.line, loc.col);
	vprintf(fmt, args);
	puts("");

	va_end(args);
}

void var_list_append(var_list* list, var v)
{
	da_append(
		(void*)&list->arr,
		&list->len,
		&list->cap,
		sizeof(var),
		&v
	);
}

static var parse_var(parser* p);
static statement parse_statement(parser* p);

// TODO: renamce
static var parser_get_list(parser* p)
{
	token tok = p->tokens.arr[p->current_token];
	var list = {0};
	if (tok.kind != TOK_BEGIN_LIST)
	{
		parser_throw(p, "pamde dev is a moron 2.0");
		return list;
	}

	p->current_token++;
	list.kind = VAR_LIST;
	while (true)
	{
		tok = p->tokens.arr[p->current_token];
		if (tok.kind == TOK_CLOSE_LIST)
		{
			p->current_token++;
			return list;
		}

		var e = parse_var(p);

		var_list_append(&list.list, e);
	}
}

static var parser_get_block(parser* p)
{
	token tok = p->tokens.arr[p->current_token];
	var block = {0};
	if (tok.kind != TOK_BEGIN_BLOCK)
	{
		parser_throw(p, "pamde dev is a moron");
		return block;
	}

	p->current_token++;
	block.kind = VAR_BLOCK;
	block.block.kind = BLOCK_PMD;
	while (true)
	{
		tok = p->tokens.arr[p->current_token];
		if (tok.kind == TOK_CLOSE_BLOCK)
		{
			return block;
		}
		if (tok.kind != TOK_ID)
		{
			parser_throw(p, "unexpected '%s'", view_token(tok));
			return block;
		}
		statement st = parse_statement(p);

		da_append (
			(void*)&block.block.pmd.sts_arr, // TODO UGILLLLLYY
			&block.block.pmd.sts_len,
			&block.block.pmd.sts_cap,
			sizeof(st),
			&st
		);
	}
}

static var parse_var(parser* p)
{
	token tok = p->tokens.arr[p->current_token];

	var v = {0};
	switch (tok.kind)
	{
	case TOK_SEMI:
	case TOK_CLOSE_LIST:
	case TOK_CLOSE_BLOCK:
	case TOK_EOF:
		return (var){0};
	case TOK_ID:
		v.kind = VAR_ATOM;
		v.atom.kind = ATOM_VAR;
		v.atom.str = tok.str;
		return v;
	case TOK_SLITERAL:
		v.kind = VAR_ATOM;
		v.atom.kind = ATOM_STRING;
		v.atom.str = tok.str;
		return v;
	case TOK_NLITERAL:
		parser_throw(p, "unimplemented: nliteral");
		return (var){0};
	case TOK_BEGIN_BLOCK:
		return parser_get_block(p);
	case TOK_BEGIN_LIST:
		return parser_get_list(p);
	}
}

static statement parse_statement(parser* p)
{
	static int counter = 0;
	counter++;
	int i = counter;
	token tok = p->tokens.arr[p->current_token];
	statement st = {0};

	if (tok.kind == TOK_EOF) return st;
	if (tok.kind != TOK_ID)
	{

		parser_throw(p, "unexpected idk");//, view_token(tok));
		printf("view_token: %s\n", view_token(tok));
		return st;
	}
	st.cmd = tok.str;

	p->current_token++;

	while (true)
	{
		tok = p->tokens.arr[p->current_token];

		if (tok.kind == TOK_EOF)
		{
			parser_throw(p, "unexpected eof");
			return st;
		}
		if (tok.kind == TOK_SEMI)
		{
			p->current_token++;
			return st; // success!
		}
		if (tok.kind == TOK_CLOSE_BLOCK)
		{
			parser_throw(p, "unexpected end of block before semicolon");
			return st;
		}
		var arg = parse_var(p);
		if (arg.kind == VAR_NULL)
		{
			parser_throw(p, "something bad hapenned");
			return st;
		}
		var_list_append(&st.args, arg);
		p->current_token++;

	}
	parser_throw(p, "this should be unreachable");
}

var create_tree(const char* source)
{
	printf(" - parsing '%s'\n", source);
	lexer lex = lexer_create(source);
	parser p = {0};
	p.lex = &lex;
	p.err = false;

	while (true)
	{
		token tok = lexer_get_token(&lex);
		/* printf("lexed token %lu '%s'\n", p.tokens.len, view_token(tok)); */
		da_append (
			(void*)&p.tokens.arr, // UGILLLLY
			&p.tokens.len,
			&p.tokens.cap,
			sizeof(token),
			&tok
		);
		if (tok.kind == TOK_EOF) break;
	}

	if (lex.err)
	{
		fprintf(
			stderr,
			"errors occured while lexing '%s'\n",
			source
		);
		return (var){0};
	}

	var tree = {0};
	tree.kind = VAR_BLOCK;
	tree.block.kind = BLOCK_PMD;

	statement st = {0};
	while ((st = parse_statement(&p)).cmd)
	{
		da_append (
			(void*)&tree.block.pmd.sts_arr,
			&tree.block.pmd.sts_len,
			&tree.block.pmd.sts_cap,
			sizeof(statement),
			&st
		);
	}

	if (p.err)
	{
		fprintf(
			stderr,
			"errors occured while parsing '%s'\n",
			source
		);
		return (var){0};
	}

	printf(" ✓ successfully parsed '%s'\n", source);
	return tree;
}

