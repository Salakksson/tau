#ifndef DA_H_
#define DA_H_

#include <stdlib.h>
#include <stdio.h>

#include <string.h>
#include <errno.h>

#define fatal(str) do {puts(str); exit(-1); } while (0)

#define DA(contents) struct { \
	contents* items; \
	int len; \
	int cap; \
}

#define da_append(list, item) \
do { \
	if ((list).len >= (list).cap) { \
		(list).cap *= 2; \
		(list).items = realloc((list).items, sizeof(*(list).items) * (list).cap); } \
	(list).items[(list).len] = item; \
	(list).len++; \
} while(0)

#define da_construct(list, sz) \
do { \
	(list).cap = sz ? sz : 10; \
	(list).len = 0; \
	(list).items = malloc(sizeof(*(list).items) * (list).cap); \
	if(!(list).items) fatal(strerror(errno));			   \
} while(0)

#endif
