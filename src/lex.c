#include "lex.h"

#include "fstring.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>



#define TOK_UNIMPLEMENTED (token){T_ERR, {.err = "UNIMPLEMETNED TOKEN"}}
#define TOK_EOF (token){lex->loc, T_EOF, {0}};

#define CHECK_EOF if (lex->ptr >= lex->sz) return TOK_EOF

#define TODO(msg) ((void)fprintf(stderr, "todo: %s", msg), (void)exit(0))

enum_map keywords[] =
{
	#define X(name, str) {K_##name, str},
	KEYWORD(X)
	#undef X
	{K_NONE, NULL},
};

enum_map puncts[] = 
{
	#define X(name, str) {P_##name, str},
	PUNCT(X)
	#undef X
	{P_NONE, NULL},
};

const char* view_keyword(keyword k)
{
	int i = 0;
	while(true)
	{
		enum_map compare = keywords[i++];
		if (compare.str == 0)
			return "INVALID";
		if (compare.e == k)
		{
			return compare.str;
		}
	}
}

const char* view_punct(punct p)
{
	int i = 0;
	while(true)
	{
		enum_map compare = puncts[i++];
		if (compare.str == 0)
			return "INVALID";
		if (compare.e == p)
		{
			return compare.str;
		}
	}
}

lexer lexer_create(char* filepath)
{
	lexer l = {0};
	
	FILE* fp = fopen(filepath, "r");
	if (!fp)
	{
		fatal("failed to open file '%s': %s", filepath, strerror(errno));
	}
	
	fseek(fp, 0, SEEK_END);
	l.sz = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	
	l.loc.path = filepath;
	l.loc.line = 1;
	l.loc.col = 1;

	l.buffer = malloc(l.sz + 1);
	size_t sum = fread(l.buffer, 1, l.sz, fp);

	if (sum != l.sz) fatal("failed to read entire file '%s': '%s'", filepath, strerror(errno));

	l.buffer[l.sz] = 0;
	
	fclose(fp);
	return l;
}

void lexer_increment_ptr(lexer* lex)
{
	if (lex->ptr  != lex->sz)
		if (lex->buffer[lex->ptr] == '\n')
		{
			lex->loc.line++;
			lex->loc.col = 1;
			lex->ptr++;
			return;
		}
	lex->ptr++;
	lex->loc.col++;
}

bool lexer_isbeginid(char c)
{
	if (c == '_') return true;
	return isalpha(c);
}

bool lexer_isinid(char c)
{
	if (c == '_') return true;
	return isalnum(c);
}

bool lexer_ispunct(char c)
{
	if (c == '_') return false;
	if (c == '\'') return false;
	if (c == '\"') return false;
	return ispunct(c);
}

int atoi_singular(char c, int base)
{
	if (!isalnum(c)) return -1;
	if ('0' <= c && c <= '9') 
	{
		int result = c - '0';
		if (result >= base) return -1;
		return result;
	}
	c = toupper(c);
	int result = c - 'A' + 10;
	if (result >= base) return -1;
	return result;
}

bool lexer_is_whitespace(lexer* lex)
{
	char c = lex->buffer[lex->ptr];
	if (lex->is_line_comment || lex->is_block_comment)
		return true;
	if (isspace(c)) return true;
	
	return false;
}

void lexer_skip_whitespace(lexer* lex)
{
	while (lexer_is_whitespace(lex))
	{
		if (lex->is_block_comment)
		{
			TODO("block comments");
		}
		if (lex->buffer[lex->ptr] == '\n') lex->is_line_comment = false;
		lexer_increment_ptr(lex);
	}
}

token lexer_get_nliteral(lexer* lex)
{
	token tok = {lex->loc, T_NLITERAL, {0}};
	char c = lex->buffer[lex->ptr];
	int base = 10;
	if (c == '0') 
	{
		lexer_increment_ptr(lex);
		CHECK_EOF;
		char base_type = lex->buffer[lex->ptr];
		switch (base_type)
		{
			case 'x': 
				base = 16;
				lexer_increment_ptr(lex);
				CHECK_EOF;
				break;
			case 'b': 
				base = 2;
				lexer_increment_ptr(lex);
				CHECK_EOF;
				break;
			default:
				if (isdigit(base_type)) base = 8;
		}
	}
	int64_t sum = 0;
	while (true)
	{
		char c = lex->buffer[lex->ptr];
		int i = atoi_singular(c, base);
		if (i < 0) break;
		sum *= base;
		sum += i;
		lexer_increment_ptr(lex);
		CHECK_EOF;
	}
	tok.value.numeric = sum;
	return tok;
}

char get_escaped_char(char c)
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
	location loc = lex->loc;
	int bufsz = 10;
	char* str = malloc(bufsz);
	int len = 0;

	lexer_increment_ptr(lex); // skip quote
	CHECK_EOF;

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
			lexer_increment_ptr(lex);
			CHECK_EOF;
			return (token){loc, T_SLITERAL, {.str = str}};
		}
		if (c == '\\')
		{
			lexer_increment_ptr(lex);
			CHECK_EOF;
			
			c = get_escaped_char(lex->buffer[lex->ptr]);
		}
		str[len] = c;
		len++;
		lexer_increment_ptr(lex);
		CHECK_EOF;
	}
}

token lexer_get_cliteral(lexer* lex)
{
	token tok = {lex->loc, T_CLITERAL, {0}};

	lexer_increment_ptr(lex); // skip quote
	CHECK_EOF;

	char c = lex->buffer[lex->ptr];
	if (c == '\\')
	{
		lexer_increment_ptr(lex);
		CHECK_EOF;
		c = lex->buffer[lex->ptr];
		// TODO: checks
		c = get_escaped_char(c);
	}
	else
	{
		c = lex->buffer[lex->ptr];
		// TODO: checks
	}
	lexer_increment_ptr(lex);
	CHECK_EOF;
	if (lex->buffer[lex->ptr] != '\'') 
	{
		return (token){lex->loc, T_ERR, {.err = fstring("no ending quote")}};
	}
	lexer_increment_ptr(lex); // skip quote
	tok.value.numeric = c;
	return tok;
}

token lexer_get_punct(lexer* lex)
{
	location loc = lex->loc;
	int bufsz = 10;
	char* str = malloc(bufsz);
	int len = 0;
	punct current_enum = P_NONE;
	while(true)
	{
		if (len + 1 >= bufsz) str = realloc(str, (bufsz *= 2));

		if (!lexer_ispunct(lex->buffer[lex->ptr])) 
		{
			free(str);
			return (token){loc, T_PUNCT, {.e = current_enum}};
		}
		str[len] = lex->buffer[lex->ptr];

		int i = 0;
		punct found_enum = P_NONE;
		while(true)
		{
			enum_map p = puncts[i++];
			if (p.str == 0) break;

			str[len + 1] = 0;
			if(!strcmp(str, p.str)) found_enum = p.e;
		}
		if (found_enum == P_NONE && current_enum != P_NONE)
		{
			free(str);
			return (token){loc, T_PUNCT, {.e = current_enum}};
		}
		current_enum = found_enum;
		len++;
		lexer_increment_ptr(lex);
		CHECK_EOF;
	}
}

token lexer_get_id(lexer* lex)
{
	location loc = lex->loc;
	int bufsz = 10;
	char* str = malloc(bufsz);
	int len = 0;
	while(true)
	{
		if (len >= bufsz) str = realloc(str, (bufsz *= 2));

		if (!lexer_isinid(lex->buffer[lex->ptr]))
		{
			str[len] = 0;
			return (token){loc, T_ID, {.str = str}};
		}
		str[len] = lex->buffer[lex->ptr];
		len++;
		lexer_increment_ptr(lex);
		CHECK_EOF;
	}
}

token lexer_get_token_raw(lexer* lex)
{
start:
	lexer_skip_whitespace(lex);
	CHECK_EOF;
	if (lex->sz - lex->ptr >= 2)
	{
		if (lex->buffer[lex->ptr] == '/')
		if (lex->buffer[lex->ptr + 1] == '/')
		{
			lex->is_line_comment = true;
			goto start;
		}
	}
	

	char c = lex->buffer[lex->ptr];
	
	if (!isprint(c))
	{
		lexer_increment_ptr(lex);
		return (token) {lex->loc, T_ERR, {.err = "Unexpected char"}};
	}
	
	if (isdigit(c)) return lexer_get_nliteral(lex);
	if (c == '\"') return lexer_get_sliteral(lex);
	if (c == '\'') return lexer_get_cliteral(lex);
	
	if (lexer_ispunct(c)) return lexer_get_punct(lex);
	
	if (lexer_isbeginid(c)) return lexer_get_id(lex);
	return (token){lex->loc, T_ERR, {.err = "if this occurs then: ☹  "}};
}

token lexer_convert_cliteral(token tok)
{
	tok.type = T_NLITERAL;
	return tok;
}

token lexer_convert_id(token tok)
{
	int i = 0;
	while(true)
	{
		enum_map k = keywords[i++];
		if (k.str == 0)
		{
			return tok;
		}
		if (!strcmp(tok.value.str, k.str))
		{
			free(tok.value.str);
			return (token){tok.loc, T_KEYWORD, {.e = k.e}};
		}
	}
}

token lexer_get_token(lexer* lex)
{
	token tok = lexer_get_token_raw(lex);
	switch(tok.type)
	{
		case T_EOF:
		case T_ERR:
		case T_PUNCT:
		case T_NLITERAL:
		case T_SLITERAL:
			return tok;
		case T_CLITERAL:
			return lexer_convert_cliteral(tok);
		case T_ID:
			return lexer_convert_id(tok);
		default:
			fatal("invalid token");
	}
	__builtin_unreachable();
}

void lexer_destroy(lexer* lex)
{
	free(lex->buffer);
}

void token_destroy(token* tok)
{
	switch(tok->type)
	{
		case T_SLITERAL:
		case T_ID:
			free(tok->value.str);
		default:
			return;
	}
}
