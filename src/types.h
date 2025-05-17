#ifndef TYPES_H_
#define TYPES_H_

#include "da.h"
#include <stdint.h>

typedef enum
{
	TYPE_INTEGER,
	TYPE_FLOAT,
	TYPE_ENUM,
	TYPE_STRUCT,
	TYPE_UNION,
	MOD_CPTR,
	MOD_PTR,
	MOD_CARR,
	MOD_ARR,
} type_kind;

typedef struct
{
	char* name;
	int32_t value;
} enumerated_value;

typedef struct type_tree
{
	type_kind kind;

	union {
		unsigned width;
		DA(enumerated_value) enums;
		DA(struct type_tree) contents;
		struct type_tree* modified_type;
	};
	char* name;
} type_tree;

inline size_t internal_sizeof(type_tree type)
{
	size_t sum = 0;
	switch (type.kind)
	{
	case TYPE_INTEGER:
		if (type.width % 8 ) fatal("invalid type width");
		return (type.width) / 8;
	case TYPE_FLOAT:
		return (type.width) / 8;
	case TYPE_ENUM:
		return (4); // TODO: custom sized enums
	case TYPE_STRUCT:
		for (int i = 0; i < type.contents.length; i++)
			sum += internal_sizeof(type.contents.items[i]);
		return sum;
	case TYPE_UNION:
		for (int i = 0; i < type.contents.length; i++)
		{
			int tmp = internal_sizeof(type.contents.items[i]);
			if (tmp > sum) sum = tmp;
		}
		return sum;
	default:
		fatal("cant find sizeof modifiers yet, or some really dusty error :/");
	}
}

#endif
