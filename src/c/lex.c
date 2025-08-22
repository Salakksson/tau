#include "lex.h"

#include "da.h"

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdbool.h>

void lexer_throw(lexer* lex, const char* msg)
{
	lex->err = true;
	printf("error while lexing %s:%lu: %s", lex->source, lex->lineno, msg);
}

void lexer_skip_whitespace(lexer* lex)
{
	while (isspace(lex->buffer[lex->ptr]))
	{
		if (lex->buffer[lex->ptr] == '\n') lex->lineno++;
		lex->ptr++;
	}
}

void cmd_append(lexer* lex, cmd* c, var v)
{
	if (!c->name)
	{
		if (v.kind != VAR_ATOM || v.atom.kind != ATOM_VAR)
		{
			// TODO: handle better and give debug info...
			lexer_throw(lex, "unknown function");
			return;
		}
		c->name = v.atom.str;
		return;
	}

	da_append(
		(void*)&c->args.vars,
		&c->args.vars_count,
		&c->args.vars_size,
		sizeof(c->args.vars[0]),
		&v
	);
}

typedef enum token_kind
{
	TOK_EOF,
	TOK_END_OF_LIST,
	TOK_END_OF_BLOCK,
	TOK_SEMI,
	TOK_VAR,
} token_kind;

typedef struct token
{
	token_kind kind;
	var v;
} token;

static char get_escaped_char(char c)
{
	switch (c)
	{
		case 'a': return '\a';
		case 'b': return '\b';
		case 'f': return '\f';
		case 'n': return '\n';
		case 'r': return '\r';
		case 't': return '\t';
		case 'v': return '\v';
		case '\\': return '\\';
		case '\'': return '\'';
		case '\"': return '\"';
		case '?': return '\?';
		default: return c;
	}
}

token lexer_get_sliteral(lexer* lex)
{
	int bufsz = 10;
	char* str = malloc(bufsz);
	int len = 0;

	lex->ptr++;
	if (!lex->buffer[lex->ptr])
		lexer_throw(lex, "unexpected eof after \"");

	// TODO: proper multiline strings
	token tok = {0};
	tok.kind = TOK_VAR;
	tok.v.kind = VAR_ATOM;
	tok.v.atom.kind = ATOM_STRING;

	while(true)
	{
		if (len >= bufsz)
		{
			str = realloc(str, (bufsz *= 2));
		}
		char c = lex->buffer[lex->ptr];
		if (c == '\"')
		{
			str[len] = 0;
			lex->ptr++;
			tok.v.atom.str = str;
			return tok;
		}
		if (c == '\\')
		{
			lex->ptr++;
			c = get_escaped_char(lex->buffer[lex->ptr]);
		}
		str[len] = c;
		len++;
		lex->ptr++;
		if (!lex->buffer[lex->ptr])
			lexer_throw(lex, "unexpected eof inside string");
	}
}

token lexer_get_token(lexer* lex);

token lexer_get_list(lexer* lex)
{
	// skip [
	lex->ptr++;
	if (!lex->buffer[lex->ptr])
		lexer_throw(lex, "unexpected eof afer [");

	token tok = {0};
	tok.kind = TOK_VAR;
	tok.v.kind = VAR_LIST;

	lexer_skip_whitespace(lex);
	while (lex->buffer[lex->ptr] != ']')
	{
		token e = lexer_get_token(lex);
		if (e.kind != TOK_VAR)
			lexer_throw(lex, "expected var");
		else
			da_append(
				(void*)&tok.v.list.vars,
				&tok.v.list.vars_count,
				&tok.v.list.vars_size,
				sizeof(tok.v.list.vars[0]),
				&e
			);
		lexer_skip_whitespace(lex);
	}

	// skip ]
	lex->ptr++;
	return tok;
}

token lexer_get_block(lexer* lex)
{
	// skip {
	lex->ptr++;
	if (!lex->buffer[lex->ptr])
		lexer_throw(lex, "unexpected eof after {");

	token tok = {0};
	tok.kind = TOK_VAR;
	tok.v.kind = VAR_BLOCK;
	tok.v.block.kind = BLOCK_PMD;
	lexer_skip_whitespace(lex);
	while (lex->buffer[lex->ptr] != '}')
	{
		cmd c = lexer_get_cmd(lex);
		da_append(
			(void*)&tok.v.block.cmds,
			&tok.v.block.cmds_count,
			&tok.v.block.cmds_size,
			sizeof(tok.v.block.cmds[0]),
			&c
		);
		lexer_skip_whitespace(lex);
		if (!lex->buffer[lex->ptr])
			lexer_throw(lex, "unexpected eof before }");
	}

	// skip }
	lex->ptr++;
	return tok;
}

bool lexer_is_end_of_id(char c)
{
	if (isspace(c)) return true;
	if (c == ';') return true;
	if (c == '"') return true;
	if (c == '[') return true;
	if (c == ']') return true;
	if (c == '{') return true;
	if (c == '}') return true;
	// WARNING: update if new puncts are added
	return false;
}

token lexer_get_id(lexer* lex)
{
	int bufsz = 10;
	char* str = malloc(bufsz);
	int len = 0;

	token tok = {0};
	tok.kind = TOK_VAR;
	tok.v.kind = VAR_ATOM;
	tok.v.atom.kind = ATOM_VAR;

	while(true)
	{
		if (len >= bufsz) str = realloc(str, (bufsz *= 2));
		// TODO: handle oom :)

		if (lexer_is_end_of_id(lex->buffer[lex->ptr]))
		{
			str[len] = 0;
			tok.v.atom.str = str;
			return tok;
		}
		str[len] = lex->buffer[lex->ptr];
		len++;
		lex->ptr++;
		if (!lex->buffer[lex->ptr])
			lexer_throw(lex, "missing trailing semicolon?");
	}
}

token lexer_get_token(lexer* lex)
{
	lexer_skip_whitespace(lex);

	char c = lex->buffer[lex->ptr];

	if (!c) return (token) {.kind = TOK_SEMI};

	if (!isprint(c))
	{
		lexer_throw(lex, "found non-ascii character, quitting");
		exit(1);
	}

	if (c == ';')
	{
		lex->ptr++;

		return (token){.kind = TOK_SEMI};
	}

	if (c == '"') return lexer_get_sliteral(lex);
	if (c == '[') return lexer_get_list(lex);
	if (c == ']') return (token){.kind = TOK_END_OF_LIST};
	if (c == '{') return lexer_get_block(lex);
	if (c == '}') return (token){.kind = TOK_END_OF_BLOCK};
	token id = lexer_get_id(lex);

	return id;
}

cmd lexer_get_cmd(lexer* lex)
{
	cmd c = {0};
	while (true)
	{
		token tok = lexer_get_token(lex);
		if (tok.kind != TOK_VAR) return c;

		cmd_append(lex, &c, tok.v);
	}
}

cmd lexer_peek_cmd(lexer* lex)
{
	lexer copy = *lex;
	return lexer_get_cmd(&copy);
}

lexer lexer_create(const char* source)
{
	lexer l = {0};

	FILE* fp = fopen(source, "r");
	if (!fp)
	{
		fprintf(stderr, "failed to open file '%s': %s", source, strerror(errno));
		exit(1);
	}

	fseek(fp, 0, SEEK_END);
	size_t sz = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	l.source = source;
	l.lineno = 1;
	l.err = false;

	char* buffer = malloc(sz + 1);
	size_t sum = fread(buffer, 1, sz, fp);

	if (sum != sz) fprintf(stderr, "failed to read entire file '%s': '%s'", source, strerror(errno));

	buffer[sz] = 0;
	l.buffer = buffer;

	fclose(fp);
	return l;
}
