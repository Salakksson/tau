#ifndef PMD_H_
#define PMD_H_

#include "tree.h"

#include <stdlib.h>
#include <stdbool.h>

#define NULL_VAR ((var){0})
#define NULL_LIST ((var_list){0})

typedef struct named_var
{
	char* name;
	var value;
} named_var;

typedef struct pmd
{
	named_var* vars;
	size_t vars_count;
	size_t vars_size;

	const char* scope;
} pmd;

void throw(const char* fn_name, const char* fmt, ...);

bool pmd_init(pmd* p);
void pmd_free(pmd* p);

pmd pmd_clone(pmd* p, const char* scope);
var pmd_eval(pmd* p, var v, const char* scope);

bool pmd_eval_bool(pmd* p, var v);

#endif
