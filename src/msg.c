#include "msg.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>

#define COLOUR_RED		"\x1b[31m"
#define COLOUR_YELLOW		"\x1b[33m"
#define COLOUR_BLUE		"\x1b[34m"
#define COLOUR_MAGENTA		"\x1b[35m"

#define COLOUR_RESET		"\x1b[0m"

#ifdef DEBUG
static msg_type log_level = MSG_DEBUG;
#else
static msg_type log_level = MSG_INFO;
#endif

void msg_set_level(msg_type level)
{
	log_level = level;
}

void msg(msg_type type, const char* message, ...)
{
	if (type < log_level) return;
	va_list args;
	va_start(args, message);
	
	const char* prefix;
	const char* colour;

	switch (type)
	{
		case MSG_FATAL:
		case MSG_ERROR:
			prefix = "ERROR: ";
			colour = COLOUR_RED;
			break;
		case MSG_WARNING:
			prefix = "WARNING: ";
			colour = COLOUR_YELLOW;
			break;
		case MSG_INFO:
			prefix = "Info: ";
			colour = COLOUR_BLUE;
			break;
		case MSG_DEBUG:
			prefix = "Debug: ";
			colour = COLOUR_MAGENTA;
			break;
		default:
			prefix = "";
			colour = COLOUR_RESET;
			break;
	}

	printf("%s%s", colour, prefix);
	vprintf(message, args);
	printf("%s\n", COLOUR_RESET);

	va_end(args);
	if (type == MSG_FATAL)
	{
		exit(1);
	}
}
