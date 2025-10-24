#include "lex.h"

#include "da.h"

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdbool.h>

// all puncts with special meaning
#define RESERVED_PUNCTS ";\"[]{}()#"

static void lexer_throw(lexer* lex, const char* fmt, ...)
{
	lex->err = true;
	va_list args;
	va_start(args, fmt);

	location loc = lex->loc;

	fprintf(stderr, "error at %s:%i:%i: ", lex->source, loc.line, loc.col);
	vprintf(fmt, args);
	puts("");

	va_end(args);
}

static void lexer_increment_ptr(lexer* lex)
{
	if (lex->ptr <= lex->sz &&
	    lex->buffer[lex->ptr] == '\n')
	{
		lex->loc.line++;
		lex->loc.col = -1;
		lex->ptr++;
		return;
	}
	lex->ptr++;
	lex->loc.col++;
}

static bool lexer_is_eof(lexer* lex)
{
	return lex->ptr >= lex->sz;
}

static void lexer_skip_whitespace(lexer* lex)
{
	while (isspace(lex->buffer[lex->ptr]))
	{
		lexer_increment_ptr(lex);
	}
}

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

static token lexer_get_sliteral(lexer* lex)
{
	int bufsz = 10;
	char* str = malloc(bufsz);
	int len = 0;

	lexer_increment_ptr(lex);
	if(lexer_is_eof(lex))
	{
		lexer_throw(lex, "unexpected eof after \"");
		return TOKEN(EOF);
	}

	// TODO: multiline strings...
	while(true)
	{
		if (len >= bufsz) str = realloc(str, (bufsz *= 2));
		char c = lex->buffer[lex->ptr];
		if (c == '\"')
		{
			lexer_increment_ptr(lex);
			break;
		}
		if (c == '\\')
		{
			lexer_increment_ptr(lex);
			if(lexer_is_eof(lex))
			{
				lexer_throw(lex, "unexpected eof inside string");
				break;
			}
			c = get_escaped_char(lex->buffer[lex->ptr]);
		}
		str[len] = c;
		len++;
		lexer_increment_ptr(lex);
		if(lexer_is_eof(lex))
		{
			lexer_throw(lex, "unexpected eof inside string");
			break;
		}

	}

	str[len] = 0;
	return TOKEN(SLITERAL, .str = str, .loc = lex->loc);
}

static void lexer_skip_comment(lexer* lex)
{
	char c;
	while ((c = lex->buffer[lex->ptr]) != '\n')
		lexer_increment_ptr(lex);
}

static token lexer_get_punct(lexer* lex)
{
	char c = lex->buffer[lex->ptr];

	switch (c)
	{
	case '"': return lexer_get_sliteral(lex);
	case '#':
		lexer_skip_comment(lex);
		return lexer_get_token(lex);
	case '[':
		lexer_increment_ptr(lex);
		return TOKEN(BEGIN_LIST, .loc = lex->loc);
	case ']':
		lexer_increment_ptr(lex);
		return TOKEN(CLOSE_LIST, .loc = lex->loc);
	case '{':
		lexer_increment_ptr(lex);
		return TOKEN(BEGIN_BLOCK, .loc = lex->loc);
	case '}':
		lexer_increment_ptr(lex);
		return TOKEN(CLOSE_BLOCK, .loc = lex->loc);
	case '(':
		lexer_increment_ptr(lex);
		return TOKEN(BEGIN_PAREN, .loc = lex->loc);
	case ')':
		lexer_increment_ptr(lex);
		return TOKEN(CLOSE_PAREN, .loc = lex->loc);
	case ';':
		lexer_increment_ptr(lex);
		return TOKEN(SEMI, .loc = lex->loc);
	default:
		lexer_throw(lex, "unreachable?");
		lexer_increment_ptr(lex);
		return TOKEN(EOF); // if this happens disaster has struck
	}
}

static bool lexer_is_end_of_id(char c)
{
	if (isspace(c)) return true;
	if (strchr(RESERVED_PUNCTS, c)) return true;

	return false;
}

static token lexer_get_id(lexer* lex)
{
	int bufsz = 10;
	char* str = malloc(bufsz);
	int len = 0;

	while(true)
	{
		if (len >= bufsz) str = realloc(str, (bufsz *= 2));
		// TODO: handle oom :)

		if (lexer_is_end_of_id(lex->buffer[lex->ptr]))
			break;

		str[len] = lex->buffer[lex->ptr];
		len++;
		lexer_increment_ptr(lex);
		if (lexer_is_eof(lex))
			break;
	}

	str[len] = 0;
	return TOKEN(ID, .str = str, .loc = lex->loc);
}

token lexer_get_token(lexer* lex)
{
	lexer_skip_whitespace(lex);
	if (lexer_is_eof(lex)) return TOKEN(EOF);

	char c = lex->buffer[lex->ptr];

	if (!isprint(c))
	{
		lexer_throw(lex, "found non-ascii character, quitting");
		exit(1);
	}

	/* if (isdigit(c)) return lexer_get_nliteral(lex); */
	// TODO: add numbers :/
	if (strchr(RESERVED_PUNCTS, c)) return lexer_get_punct(lex);

	return lexer_get_id(lex);
}

lexer lexer_create(const char* source)
{
	lexer l = {0};

	FILE* fp = fopen(source, "r");
	if (!fp)
	{
		fprintf(
			stderr,
			"failed to open file '%s': %s",
			source,
			strerror(errno)
		);
		exit(1);
	}

	fseek(fp, 0, SEEK_END);
	l.sz = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	l.source = source;
	l.loc.line = 1;
	l.loc.col = 0;
	l.err = false;

	char* buffer = malloc(l.sz + 1);
	size_t sum = fread(buffer, 1, l.sz, fp);

	if (sum != l.sz) fprintf(
		stderr,
		"failed to read entire file '%s': '%s'",
		source,
		strerror(errno)
	);

	buffer[l.sz] = 0;
	l.buffer = buffer;

	fclose(fp);
	return l;
}

char* view_token(token tok)
{
	switch (tok.kind)
	{
	case TOK_BEGIN_BLOCK: return strdup("{");
	case TOK_BEGIN_PAREN: return strdup("(");
	case TOK_BEGIN_LIST:  return strdup("[");
	case TOK_CLOSE_BLOCK: return strdup("}");
	case TOK_CLOSE_PAREN: return strdup(")");
	case TOK_CLOSE_LIST:  return strdup("]");
	case TOK_SEMI:        return strdup(";");
	case TOK_EOF:         return strdup("eof");
	case TOK_ID:          return strdup(tok.str);
	case TOK_NLITERAL:    return fstring("%lf", tok.number);
	case TOK_SLITERAL:    return fstring("\"%s\"", tok.str);
	default:              return strdup("?!?");
	}
}
