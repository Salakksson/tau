#ifndef MSG_H_
#define MSG_H_

#include <stddef.h>

typedef enum
{
	MSG_DEBUG,
	MSG_INFO,
	MSG_WARNING,
	MSG_ERROR,
	MSG_FATAL,
	MSG_TODO,
} msg_kind;

#define LOC_INTERNAL (location){ __FILE__, __LINE__, 0}

void msg_set_level(msg_kind level);
void msg(msg_kind type, const char* message, ...);

// Code errors
#define err(str, ...) msg(MSG_ERROR, str, ##__VA_ARGS__)
#define warn(str, ...) msg(MSG_WARNING, str, ##__VA_ARGS__)

#define __NORETURN__(statement) do {statement; __builtin_unreachable();} while(0)

// Internal errors
#define fatal(str, ...) __NORETURN__(msg(MSG_FATAL, str, ##__VA_ARGS__))
#define todo(str, ...) __NORETURN__(msg(MSG_TODO, str, ##__VA_ARGS__))
#define info(str, ...) msg(MSG_INFO, str, ##__VA_ARGS__)
#define debug(str, ...) msg(MSG_DEBUG, str, ##__VA_ARGS__)

#undef assert
#define assert(cond, ...) ( \
	(cond) ? (void)0 \
	: diag(DIAG_FATAL, \
"Assertion '" #cond "' failed - " __VA_ARGS__) \
)

#endif
