#include "da.h"
#include <stdint.h>

typedef enum
{
	TYPE_INTEGER,
	TYPE_FLOAT,
	TYPE_ENUM,
	TYPE_MODIFIER,
	TYPE_STRUCT,
	TYPE_UNION,
} type_kind;

typedef struct
{
	char* name;
	int32_t value;
} enumerated_value;

typedef struct type_tree type_tree;
typedef struct
{
	type_tree* type;
} modified_type;

struct type_tree
{
	type_kind kind;

	union{
		unsigned width;
		DA(enumerated_value) enums;
		DA(struct type_tree) contents;
		modified_type modifier;
	};
	char* name;
};

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
		case TYPE_MODIFIER:
			fatal("TODO: sizeof modifier");
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
	}
}
