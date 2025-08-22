#ifndef PMD_H_
#define PMD_H_

#include "lex.h"

#include <stdlib.h>
#include <stdbool.h>

#define NULL_VAR ((var){0})
#define NULL_LIST ((var_list){0})

typedef struct named_var
{
	const char* name;
	var_list args;
	var value;
} named_var;

typedef struct pmd
{
	named_var* vars;
	size_t vars_count;
	size_t vars_size;
} pmd;

void throw(const char* fn_name, const char* message);

char* var_to_str(pmd* p, var v);
bool pmd_init(pmd* p);
void pmd_free(pmd* p);
bool pmd_source(pmd p, const char* source);

#endif
