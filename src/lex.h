#ifndef LEX_H_
#define LEX_H_

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include "diag.h"

#include "x_macros.h"

typedef enum
{
	K_NONE = 0,
	#define X(name, str) K_##name,
	KEYWORD(X)
	#undef X
} keyword;

typedef enum
{
	P_NONE = 0,
	#define X(name, str) P_##name,
	PUNCT(X)
	#undef X
} punct;

typedef struct
{
	int e;
	const char* str;
} enum_map;

typedef enum
{
	T_EOF = 0,
	T_GET_NEXT,
	T_ID,
	T_PUNCT,
	T_NLITERAL,
	T_SLITERAL,
	T_KEYWORD,
} token_kind;

typedef struct
{
	location loc;

	token_kind type;

	union {
		int e;
		uint64_t numeric;
		char* str;
	};

} token;

typedef struct
{
	const char* buffer;
	size_t sz;
	size_t ptr;

	location loc;

	bool is_line_comment;
	bool is_block_comment;

} lexer;

const char* view_keyword(keyword);
const char* view_punct(punct);
const char *view_token(token);

lexer lexer_create(char* source);

token lexer_get_token(lexer* lex);

token lexer_peek_token(lexer* lex);

void lexer_destroy(lexer* lex);

void token_destroy(token* tok);

#endif
