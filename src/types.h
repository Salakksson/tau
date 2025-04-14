#include "da.h"
#include <stdint.h>

typedef enum
{
	TYPE_INTEGER,
	TYPE_FLOAT,
	TYPE_ENUM,
	TYPE_STRUCT,
	TYPE_UNION,
	TYPE_POINTER,
} type_kind;

typedef struct
{
	char* name;
	int32_t value;
} enumerated_value;

typedef struct
{
	int todo;
}pointer_info;

da_declare(enum_list, enumerated_value);

typedef struct type type;
da_declare(type_list, type);

struct type
{
	type_kind kind;
	
	union{
		unsigned width;
		enum_list enums;
		type_list contents;
		pointer_info ptr;
	};
	
	char* name;
};
