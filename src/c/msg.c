#include "msg.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>

#define COLOR_RED     "\x1b[31m"
#define COLOR_YELLOW  "\x1b[33m"
#define COLOR_BLUE    "\x1b[34m"
#define COLOR_MAGENTA "\x1b[35m"

#define COLOR_RESET   "\x1b[0m"

static msg_kind log_level = MSG_INFO;

void msg_set_level(msg_kind level)
{
	log_level = level;
}

void msg(msg_kind type, const char* message, ...)
{
	if (type < log_level) return;

	va_list args;
	va_start(args, message);

	const char* prefix;
	const char* color;

	switch (type)
	{
	case MSG_FATAL:
	case MSG_ERROR:
		prefix = " × ERROR: ";
		color = COLOR_RED;
		break;
	case MSG_TODO:
		prefix = " ☹ TODO: ";
		color = COLOR_RED;
		break;
	case MSG_WARNING:
		prefix = " ~ WARNING: ";
		color = COLOR_YELLOW;
		break;
	case MSG_INFO:
		prefix = " - Info: ";
		color = COLOR_BLUE;
		break;
	case MSG_DEBUG:
#ifdef DEBUG
		prefix = " ? Debug: ";
		color = COLOR_MAGENTA;
		break;
#else
		return;
#endif
	default:
		prefix = "";
		color = COLOR_RESET;
		break;
	}
	size_t size = 0;
	va_list check_size;
	va_copy(check_size, args);

	size += snprintf(0, 0, "%s%s", color, prefix);
	size += vsnprintf(0, 0, message, check_size);
	size += snprintf(0, 0, "%s\n", COLOR_RESET);

	char* msg = malloc(size + 1);
	char* ptr = msg;

	ptr += sprintf(ptr, "%s%s", color, prefix);
	ptr += vsprintf(ptr, message, args);
	ptr += sprintf(ptr, "%s", COLOR_RESET);

	va_end(args);
	va_end(check_size);

	puts(msg);
	free(msg);

	if (type == MSG_FATAL || type == MSG_TODO)
		exit(1);

	return;
 }
