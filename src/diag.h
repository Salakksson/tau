#ifndef DIAG_H_
#define DIAG_H_

typedef enum
{
	DIAG_DEBUG,
	DIAG_INFO,
	DIAG_WARNING,
	DIAG_ERROR,
	DIAG_FATAL
} diag_type;

void diag_set_level(diag_type level);
void diag(diag_type type, const char* message, ...);

#define _STRING_LITERAL(a) #a
#define STRING_LITERAL(a) _STRING_LITERAL(a)

#define FILE_LINE() __FILE__ ":" STRING_LITERAL(__LINE__)

#define err(str, ...) diag(DIAG_ERROR, str " - " FILE_LINE(), ##__VA_ARGS__)
#define fatal(str, ...) diag(DIAG_FATAL, str " - " FILE_LINE(), ##__VA_ARGS__)
#define warn(str, ...) diag(DIAG_WARNING, str " - " FILE_LINE(), ##__VA_ARGS__)

#define todo(str, ...) diag(DIAG_TODO, str " - " FILE_LINE(), ##__VA_ARGS__)

#undef assert
#define assert(cond, ...) ( \
	(cond) ? (void)0 \
		 : diag(DIAG_FATAL, \
"Assertion '" #cond "' failed - " FILE_LINE() ": " __VA_ARGS__) \
)


#endif
