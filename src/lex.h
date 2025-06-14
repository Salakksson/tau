#ifndef LEX_H_
#define LEX_H_

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include "msg.h"

#include "x_macros.h"

typedef enum
{
	K_NONE = 0,
	#define X(name, str) K_##name,
	KEYWORD(X)
	#undef X
} keyword;

const char* view_keyword(keyword);

typedef enum
{
	P_NONE = 0,
	#define X(name, str) P_##name,
	PUNCT(X)
	#undef X
} punct;

const char* view_punct(punct);

typedef struct
{
	int e;
	const char* str;
} enum_map;

typedef struct
{
	char* path;
	int line;
	int col;
} location;

typedef struct
{
	location loc;

	enum {
		T_EOF = 0,
		T_ERR,
		T_ID,
		T_PUNCT,
		T_NLITERAL,
		T_SLITERAL,
		T_CLITERAL,
		T_KEYWORD,
	} type;
	
	union {
		int e;
		uint64_t numeric;
		char* str;
		const char* err;
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

lexer lexer_create(char* source);

token lexer_get_token(lexer* lex);

token lexer_peek_token(lexer* lex);

void lexer_destroy(lexer* lex);

void token_destroy(token* tok);

#endif
