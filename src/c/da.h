#ifndef DA_H_
#define DA_H_

#include <stdlib.h>
#include <stdio.h>

#include <string.h>
#include <errno.h>
#include <stdarg.h>

#include "msg.h"

#define DA_DEFAULT_CAP 4

static inline void da_append(void** list, size_t* len, size_t* cap, size_t sz, void* item)
{
	if (!list) fatal("da_append: invalid 'list'");
	if (!len) fatal("da_append: invalid 'len'");
	if (!cap) fatal("da_append: invalid 'cap'");

	if (!*list)
	{
		if (*cap) fatal("da_append: mutilated da");
		if (*len) fatal("da_append: mutilated da");
		*cap = DA_DEFAULT_CAP;
		*list = malloc(sz * *cap);
		if (!*list) fatal("da_append: oom");
	}
	if (*cap <= *len)
	{
		if (*cap <= 0) fatal("da_append: mutilated da");
		if (*len <= 0) fatal("da_append: mutilated da");
		*cap *= 2;
		*list = realloc(*list, sz * *cap);
		if (!*list) fatal("da_append: oom");
	}

	memcpy((char*)*list + *len * sz, item, sz);
	(*len)++;
}

static inline char* vfstring(const char* fmt, va_list args)
{
	va_list copy;
	va_copy(copy, args);

	size_t sz = vsnprintf(0, 0, fmt, args);
	char* str = malloc(sz + 1);
	vsnprintf(str, sz + 1, fmt, copy);

	va_end(copy);

	return str;
}

static inline char* fstring(const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);

	char* str = vfstring(fmt, args);
	va_end(args);

	return str;
}

#endif
