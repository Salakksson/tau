#ifndef LEX_H_
#define LEX_H_

#include <stdlib.h>
#include <stdbool.h>

typedef enum token_kind
{
	TOK_EOF,
	TOK_BEGIN_LIST,
	TOK_CLOSE_LIST,
	TOK_BEGIN_BLOCK,
	TOK_CLOSE_BLOCK,
	TOK_SEMI,
	TOK_SLITERAL,
	TOK_NLITERAL,
	TOK_ID,
} token_kind;

typedef struct location
{
	int line;
	int col;
} location;

typedef struct token
{
	token_kind kind;
	union {
		char* str;
		double number;
	};
	location loc;
} token;

#define TOKEN(KIND, ...) ((token){.kind = TOK_##KIND ,##__VA_ARGS__})

typedef struct lexer
{
	const char* source;

	char* buffer;
	size_t ptr;
	size_t sz;

	location loc;

	bool err;
} lexer;

char* view_token(token tok);

lexer lexer_create(const char* source);

token lexer_get_token(lexer* lex);
token lexer_peek_token(lexer* lex);

#endif
