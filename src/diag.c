#include "diag.h"

#include "da.h"

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

static diag_kind log_level = DIAG_INFO;

void diag_set_level(diag_kind level)
{
	log_level = level;
}

static DA(diagnostic) diags;

static void diag_push(diagnostic d)
{
	da_append(diags, d);
}

void diag(diag_kind type, location loc, const char* message, ...)
{
	if (type < log_level) return;

	va_list args;
	va_start(args, message);

	const char* prefix;
	const char* color;

	switch (type)
	{
	case DIAG_FATAL:
		prefix = " × ERROR: ";
		color = COLOR_RED;
		break;
	case DIAG_TODO:
		prefix = " ☹ TODO: ";
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
	size_t size = 0;
	va_list check_size;
	va_copy(check_size, args);

	size += snprintf(0, 0, "%s:%i:%i ", loc.path, loc.line, loc.col);
	size += snprintf(0, 0, "%s%s", color, prefix);
	size += vsnprintf(0, 0, message, check_size);
	size += snprintf(0, 0, "%s\n", COLOR_RESET);

	char* msg = malloc(size + 1);
	char* ptr = msg;

	diagnostic d = {0};
	ptr += sprintf(ptr, "%s:%i:%i", loc.path, loc.line, loc.col);

	d.msg = ptr;
	d.loc = loc;
	ptr += sprintf(ptr, "%s%s", color, prefix);
	ptr += vsprintf(ptr, message, args);
	ptr += sprintf(ptr, "%s", COLOR_RESET);

	va_end(args);
	va_end(check_size);

	if (type == DIAG_FATAL || type == DIAG_TODO)
	{
		puts(msg);
		exit(1);
	}
	else if (type == DIAG_INFO)
	{
		puts(msg);
		free(msg);
		return;
	}

	diag_push(d);
 }
