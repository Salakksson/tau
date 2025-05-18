#ifndef DIAG_H_
#define DIAG_H_

#include <stddef.h>

typedef enum
{
	DIAG_DEBUG,
	DIAG_INFO,
	DIAG_WARNING,
	DIAG_ERROR,
	DIAG_FATAL,
	DIAG_TODO,
} diag_kind;

typedef struct
{
	char* path;
	int line;
	int col;
} location;

#define LOC_INTERNAL (location){ __FILE__, __LINE__, 0}

typedef struct
{
	char* msg;
	location loc;
} diagnostic;

void diag_set_level(diag_kind level);
void diag(diag_kind type, location loc, const char* message, ...);

// Code errors
#define err(loc, str, ...) diag(DIAG_ERROR, loc, str, ##__VA_ARGS__)
#define warn(loc, str, ...) diag(DIAG_WARNING, loc, str, ##__VA_ARGS__)

#define __NORETURN__(statement) do {statement; __builtin_unreachable();} while(0)

// Internal errors
#define fatal(str, ...) __NORETURN__(diag(DIAG_FATAL, LOC_INTERNAL, str, ##__VA_ARGS__))
#define todo(str, ...) __NORETURN__(diag(DIAG_TODO, LOC_INTERNAL, str, ##__VA_ARGS__))
#define info(str, ...) diag(DIAG_INFO, LOC_INTERNAL, str, ##__VA_ARGS__)

#undef assert
#define assert(cond, ...) ( \
	(cond) ? (void)0 \
	: diag(DIAG_FATAL, LOC_INTERNAL \
"Assertion '" #cond "' failed - " __VA_ARGS__) \
)

#endif
