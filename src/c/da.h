#ifndef DA_H_
#define DA_H_

#include <stdlib.h>
#include <stdio.h>

#include <string.h>
#include <errno.h>

#define fatal(str) do {puts(str); exit(-1); } while (0)
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

/* #define DA(contents) struct { \ */
/*	contents* items; \ */
/*	int len; \ */
/*	int cap; \ */
/* } */

/* #define da_append(list, item) \ */
/* do { \ */
/*	if ((list).len >= (list).cap) { \ */
/*		(list).cap *= 2; \ */
/*		(list).items = realloc((list).items, sizeof(*(list).items) * (list).cap); } \ */
/*	(list).items[(list).len] = item; \ */
/*	(list).len++; \ */
/* } while(0) */

/* #define da_construct(list, sz) \ */
/* do { \ */
/*	(list).cap = sz ? sz : 10; \ */
/*	(list).len = 0; \ */
/*	(list).items = malloc(sizeof(*(list).items) * (list).cap); \ */
/*	if(!(list).items) fatal(strerror(errno)); \ */
/* } while(0) */

/* #define da_delete(list) free((list).items) */

/* #define da_view_str(list, buf) \ */
/* do { \ */
/*	size_t sz = 0; \ */
/*	sz += snprintf(0, 0, "{"); \ */
/*	for (int i = 0; i < (list).len; i++) \ */
/*		sz += snprintf(0, 0, ", \"%s\"", (list).items[i]); \ */
/*	sz += snprintf(0, 0, "}"); \ */
/* \ */
/*	(buf) = malloc(sz + 1);   \ */
/*	char* end = (buf) + sz + 1; \ */
/*	(buf) += snprintf((buf), end - (buf), "{"); \ */
/*	for (int i = 0; i < (list).len; i++) \ */
/*	{ \ */
/*		const char* format = (i == 0) ? "\"%s\"" : ", \"%s\""; \ */
/*		(buf) += snprintf((buf), end - (buf), format, (list).items[i]); \ */
/*	} \ */
/*	(buf) += snprintf((buf), end - (buf), "}"); \ */
/* } while (0); */

/* #define da_swap(list, a, b) \ */
/* do { \ */
/*	char* tmp = (list).items[a]; \ */
/*	(list).items[a] = (list).items[b]; \ */
/*	(list).items[b] = tmp; \ */
/* } while (0); */

/* #define da_sort_string(list) \ */
/* do { \ */
/*	for (int i = 0; i < (list).len - 1; i++) \ */
/*	{ \ */
/*		for (int j = 0; j < (list).len - 1 - i; j++) \ */
/*		{ \ */
/*			if (strcmp((list).items[j], (list).items[j+1]) > 0 ) \ */
/*				da_swap(list, j, j + 1); \ */
/*		} \ */
/*	} \ */
/* \ */
/* } while (0); */

#endif
