#ifndef DA_H_
#define DA_H_

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
}while


#endif
