#ifndef MSG_H_
#define MSG_H_

typedef enum msg_type
{
	MSG_DEBUG,
	MSG_INFO,
	MSG_WARNING,
	MSG_ERROR,
	MSG_FATAL,
} msg_type;

void msg_set_level(msg_type level);
void msg(msg_type type, const char* message, ...);

#define _STRING_LITERAL(a) #a
#define STRING_LITERAL(a) _STRING_LITERAL(a)

#define FILE_LINE() __FILE__ ":" STRING_LITERAL(__LINE__)

#define err(str, ...) msg(MSG_ERROR, str " - " FILE_LINE(), ##__VA_ARGS__)
#define fatal(str, ...) msg(MSG_FATAL, str " - " FILE_LINE(), ##__VA_ARGS__)
#define dbg(str, ...) msg(MSG_DEBUG, str " - " FILE_LINE(), ##__VA_ARGS__)

#define UNIMPLEMENTED(str) FATAL("Unimplemented code reached: '%s'", str)

#undef assert
#define assert(cond, ...) ( \
	(cond) ? (void)0    \
		 : msg(MSG_FATAL, \
"Assertion '" #cond "' failed - " FILE_LINE() ": " __VA_ARGS__)\
)

#endif
