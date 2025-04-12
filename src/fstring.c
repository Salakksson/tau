#include "fstring.h"

#include "msg.h"

#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>

char* fstring(const char* fmt, ...)
{
	static char buffer[1024] = {0};
	static int ptr = 0;
	static int sz = sizeof(buffer);

	va_list args;
	va_list args_copy;
	va_start(args, fmt);
	va_copy(args_copy, args);

	int text_sz = snprintf(0, 0, fmt, args);
	if (ptr + text_sz + 1 >= sz)
	{
		ptr = 0;
		if (text_sz + 1 >= sz)
		{
			fatal("text to fatt for buffer bruh");
		}
	}
	int written_sz = snprintf(buffer + ptr, text_sz, fmt, args_copy);
	assert(written_sz == text_sz);

	va_end(args);

	return buffer + ptr;
}
