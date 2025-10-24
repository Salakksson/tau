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

#define _STRINGIFY(x) #x
#define _TO_STRING(x) _STRINGIFY(x)

#define _LOCATION __FILE__ ":" _TO_STRING(__LINE__) ": "

void msg_set_level(msg_kind level);
void msg(msg_kind type, const char* message, ...);

#define __NORETURN__(statement) do {statement; __builtin_unreachable();} while(0)

#define err(str, ...) msg(MSG_ERROR, str, ##__VA_ARGS__)
#define warn(str, ...) msg(MSG_WARNING, str, ##__VA_ARGS__)
#define fatal(str, ...) __NORETURN__(msg(MSG_FATAL, _LOCATION str, ##__VA_ARGS__))
#define todo(str, ...) __NORETURN__(msg(MSG_TODO, _LOCATION str, ##__VA_ARGS__))
#define debug(str, ...) msg(MSG_DEBUG, _LOCATION str, ##__VA_ARGS__)
#define info(str, ...) msg(MSG_INFO, str, ##__VA_ARGS__)

#undef assert
#define assert(cond, ...) ( \
	(cond) ? (void)0 \
	: diag(DIAG_FATAL, LOC_INTERNAL \
"Assertion '" #cond "' failed - " __VA_ARGS__) \
)

#endif
