#ifndef DA_H_
#define DA_H_

#include "msg.h"
#include <stdlib.h>

#define da_declare(name, contents)\
typedef struct\
{\
	contents* items;\
	int length;\
	int capacity;\
} name;

#define da_append(list, item)\
do {\
	if (list.length >= list.capacity) realloc(list.items, list.capacity *= 2); \
	list.items[list.length] = item;\
	list.length++;\
} while(0)

#define da_construct(list, sz)\
do{\
	list.capacity = sz ? sz : 10;\
	list.items = malloc(list.capacity);\
	if(!list.items) fatal("failed to malloc in da_contruct macro");\
} while(0)

#endif
