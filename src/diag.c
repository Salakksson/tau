#include "diag.h"

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

static diag_type log_level = DIAG_INFO;

void diag_set_level(diag_type level)
{
	log_level = level;
}

void diag(diag_type type, const char* message, ...)
{
	if (type < log_level) return;
	va_list args;
	va_start(args, message);

	const char* prefix;
	const char* color;

	switch (type)
	{
		case DIAG_FATAL:
			prefix = " × INTERNAL ERROR: ";
			color = COLOR_RED;
			break;
		case DIAG_ERROR:
			prefix = " × ERROR: ";
			color = COLOR_RED;
			break;
		case DIAG_WARNING:
			prefix = " ~ WARNING: ";
			color = COLOR_YELLOW;
			break;
		case DIAG_INFO:
			prefix = " - Info: ";
			color = COLOR_BLUE;
			break;
		case DIAG_DEBUG:
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

	printf("%s%s", color, prefix);
	vprintf(message, args);
	printf("%s\n", COLOR_RESET);

	va_end(args);
	if (type == DIAG_FATAL) exit(1);
}
