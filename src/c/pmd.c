#include "pmd.h"

#include "da.h"

#include <stdio.h>
#include <string.h>
#include <stdarg.h>

static bool g_thrown = false;

void* memdup(const void* src, size_t sz)
{
	void* dst = malloc(sz);
	if (!dst) return dst;
	memcpy(dst, src, sz);
	return dst;
}

var copy_var(var v)
{
	// TODO in future check performance diff of
	// cutting memdup'd blocks to count instead of size
	switch (v.kind)
	{
	case VAR_ATOM:
		if (v.atom.kind != ATOM_NUMBER)
			v.atom.str = strdup(v.atom.str);
		break;
	case VAR_LIST:
	{
		var* new_vars = memdup(
			v.list.vars,
			v.list.vars_size
			* sizeof(var)
		);
		for (size_t i = 0; i < v.list.vars_count; i++)
			new_vars[i] = copy_var(new_vars[i]);
		v.list.vars = new_vars;
		break;
	}
	case VAR_BLOCK:
	{
		if (v.block.kind != BLOCK_PMD) break;
		cmd* new_cmds = memdup(
			v.block.cmds,
			v.block.cmds_size
			* sizeof(cmd)
		);
		for (size_t i = 0; i < v.block.cmds_count; i++)
		{
			cmd c = new_cmds[i];
			var old = {.kind = VAR_LIST, .list = c.args};
			var new = copy_var(old);
			new_cmds[i].args = new.list;
		}
		v.block.cmds = new_cmds;
		break;
	}
	default:
		break;
	}
	return v;
}

void define(pmd* p, named_var def)
{
	def.name = strdup(def.name);
	def.value = copy_var(def.value);
	da_append(
		(void*)&p->vars,
		&p->vars_count,
		&p->vars_size,
		sizeof(p->vars[0]),
		&def
	);
}

named_var pmd_get_named_var(pmd* p, const char* name)
{
	for (size_t i = 0; i < p->vars_count; i++)
	{
		named_var v = p->vars[i];

		if (!strcmp(v.name, name)) return v;
	}
	return (named_var){0};
}

void throw(const char* fn_name, const char* message)
{
	fprintf(stderr, "encountered error: '%s' in %s\n", message, fn_name);
	g_thrown = true;
}

bool handle_stack_trace(const char* name)
{
	if (!g_thrown) return false;

	fprintf(stderr, "trace: '%s'\n", name);
	return true;
}

void var_list_append(var_list* list, var v)
{
	da_append(
		(void*)&list->vars,
		&list->vars_count,
		&list->vars_size,
		sizeof(list->vars[0]),
		&v
	);
}

char* fstring(const char* fmt, ...)
{
	va_list args;
	va_list copy;
	va_start(args, fmt);
	va_copy(copy, args);

	size_t sz = vsnprintf(0, 0, fmt, args);
	char* str = malloc(sz + 1);
	vsnprintf(str, sz + 1, fmt, copy);

	va_end(copy);
	va_end(args);

	return str;
}

var evaluate_shell_block(pmd* p, const char* shellscript)
{
	throw("evaluate_shell_block", "unimplemented");
	return NULL_VAR;
}

var evaluate_function_call(pmd* p, var_list args, var block, const char* name)
{
	// block should be a block but can be anything
	if (block.kind != VAR_BLOCK) return block;

	var_block b = block.block;
	if (b.kind == BLOCK_C) return b.c(p, args);
	if (b.kind == BLOCK_SHELL) return evaluate_shell_block(p, b.shell);
	for (size_t i = 0; i < b.cmds_count; i++)
	{
		cmd statement = b.cmds[i];
		if (!strcmp(statement.name, "return"))
		{
			if (statement.args.vars_count == 1)
				return statement.args.vars[0];
			return (var) {
				.kind = VAR_LIST,
				.list = statement.args
			};
		}

		named_var func = pmd_get_named_var(p, statement.name);
		if (!func.name)
		{
			// TODO: log the name when throw gets an upgrade
			throw(statement.name, "undefined function");
			return NULL_VAR;
		}
		printf("calling '%s'\n", func.name);
		evaluate_function_call(p, statement.args, func.value, func.name);
		if (handle_stack_trace(name)) return NULL_VAR;
	}
	// this is a sucess if theres no return value, void returns empty list
	return NULL_VAR;
}

var evaluate_var(pmd* p, var v)
{
	if (v.kind != VAR_BLOCK) return v;

	return evaluate_function_call(p, NULL_LIST, v, "anonymous");
}

var evaluate_named_var(pmd* p, named_var v)
{
	if (!v.name) return NULL_VAR;
	if (v.value.kind != VAR_BLOCK)
		return evaluate_var(p, v.value);

	return evaluate_function_call(p, v.args, v.value, v.name);
}

bool evaluate_condition(pmd* p, var cond)
{
	if (cond.kind == VAR_BLOCK) cond = evaluate_var(p, cond);
	if (cond.kind == VAR_LIST) return cond.list.vars_count > 0;
	if (cond.kind != VAR_ATOM) return false;

	var_atom atom = cond.atom;

	if (atom.kind == ATOM_VAR)
	{
		named_var v = pmd_get_named_var(p, atom.str);
		var eval = evaluate_named_var(p, v);
		return evaluate_condition(p, eval);
	}
	if (atom.kind == ATOM_STRING)
		return atom.str;
	if (atom.kind == ATOM_NUMBER)
		return atom.value;

	throw("evaluate_condition", "this should be unreachable");
	return false;

}

char* var_to_str(pmd* p, var v)
{
	switch (v.kind)
	{
	case VAR_ATOM:
		if (v.atom.kind == ATOM_NUMBER)
			return fstring("%d", v.atom.value);
		if (v.atom.kind == ATOM_STRING)
			return fstring("\"%s\"", v.atom.str);

		named_var func = pmd_get_named_var(p, v.atom.str);
		var eval = evaluate_named_var(p, func);
		return var_to_str(p, eval);
	case VAR_BLOCK:
		return strdup("kind=block");
	case VAR_LIST:
		return strdup("kind=list");
	default:
		return strdup("null");
	}
}

var impl_echo(pmd* p, var_list args)
{
	for (size_t i = 0; i < args.vars_count; i++)
	{
		var arg = args.vars[i];
		if (arg.kind == VAR_ATOM && arg.atom.kind == ATOM_STRING)
		{
			puts (arg.atom.str);
			break;
		}
		char* str = var_to_str(p, args.vars[i]);
		puts(str);
		free(str);
	}
	return NULL_VAR;
}

var impl_def(pmd* p, var_list args)
{
	if (args.vars_count < 3)
	{
		throw("def", "not enough args");
		return NULL_VAR;
	}

	bool found_eq = false;
	size_t i;
	named_var def = {0};

	for (i = 0; i < args.vars_count; i++)
	{
		var arg = args.vars[i];
		if (arg.kind != VAR_ATOM)
		{
			// TODO: when throw gets upgraded print the token here
			// TODO: also handle this shit better
			throw("def", "undexpected non-atom");
			return NULL_VAR;

		}

		var_atom atom = arg.atom;

		if (atom.kind == ATOM_VAR)
		{
			if (!strcmp(atom.str, "="))
			{
				found_eq = true;
				i++;
				break;
			}
			if (!def.name)
				def.name = atom.str;
			else
				var_list_append(&def.args, arg);
			continue;
		}

		throw("def", "string or number encountered before =");
		return NULL_VAR;
	}

	if (!found_eq)
	{
		throw("def", "missing '=' argument");
		return NULL_VAR;
	}

	if (!def.name)
	{
		throw("def", "missing variable name");
		return NULL_VAR;
	}

	int n_args_left = args.vars_count - i;
	if (n_args_left != 1)
	{
		printf("%d\n", n_args_left);
		for (int j = 0; j < n_args_left; j++)
		{
			printf("%i: %s\n", j, args.vars[i+j].atom.str);
		}
		throw("def", "more than one arg after '=");
		return NULL_VAR;
	}
	def.value = args.vars[i];
	define(p, def);
	return NULL_VAR;
}

var impl_if (pmd* p, var_list args)
{
	if (args.vars_count != 2)
	{
		throw("if", "if takes two arguments: `if <cond> <value>`");
		return NULL_VAR;
	}
	var cond = args.vars[0];
	var value = args.vars[1];
	bool eval = evaluate_condition(p, cond);
	if (eval)
		return evaluate_var(p, value);
	else
		return NULL_VAR;
}

void def_c_func(pmd* p, const char* name, c_func f)
{
	named_var def = {0};
	def.name = name;
	def.value.kind = VAR_BLOCK;
	def.value.block.kind = BLOCK_C;
	def.value.block.c = f;
	define(p, def);
}

void def_number(pmd* p, const char* name, int value)
{
	named_var def = {0};
	def.name = name;
	def.value.kind = VAR_ATOM;
	def.value.atom.kind = ATOM_NUMBER;
	def.value.atom.value = value;
	define(p, def);
}

void def_string(pmd* p, const char* name, const char* str)
{
	named_var def = {0};
	def.name = name;
	def.value.kind = VAR_ATOM;
	def.value.atom.kind = ATOM_STRING;
	def.value.atom.str = (char*)str;
	define(p, def);
}

bool pmd_init(pmd* p)
{
	p->vars_count = 0;
	p->vars_size = 40;
	p->vars = malloc(sizeof(var) * p->vars_size);

	def_c_func(p, "def", impl_def);
	def_c_func(p, "if", impl_if);
	def_c_func(p, "echo", impl_echo);

	def_number(p, "false", 0);
	def_number(p, "true", 1);
	return true;
}

void pmd_free(pmd* p)
{
	free(p->vars);
}

void free_var(var v)
{
	switch (v.kind)
	{
	case VAR_ATOM:
		if (v.atom.kind != ATOM_NUMBER)
		{
			printf("freeing string '%s'\n", v.atom.str);
			free(v.atom.str);
		}
		break;
	case VAR_LIST:
		for (size_t i = 0; i < v.list.vars_count; i++)
			free_var(v.list.vars[i]);
		printf("freeing list: %p\n", v.list.vars);
		free(v.list.vars);
		break;
	case VAR_BLOCK:
		for (size_t i = 0; i < v.block.cmds_count; i++)
		{
			cmd c = v.block.cmds[i];
			var tmp = {.kind = VAR_LIST, .list = c.args};
			free_var(tmp);
		}
		printf("freeing block: %p\n", v.block.cmds);
		free(v.block.cmds);
		break;
	default:
		return;
	}
}

bool pmd_source(pmd p, const char* source)
{
	lexer lex = lexer_create(source);

	int i = 0;
	while (true)
	{
		printf("cmd %i:\n", i++);
		cmd c = lexer_get_cmd(&lex);
		if (lex.err) {
			printf("lexing error\n");
			free(lex.buffer);
			return false;
		}
		if (!c.name) break;
		var block = {0};
		block.kind = VAR_BLOCK;
		block.block.kind = BLOCK_PMD;
		da_append(
			(void*)&block.block.cmds,
			&block.block.cmds_count,
			&block.block.cmds_size,
			sizeof(block.block.cmds[0]),
			&c
		);
		evaluate_function_call(&p, (var_list){0}, block, "::");
		free(block.block.cmds);
		var list_to_free = (var){.kind = VAR_LIST, .list = c.args};
		free_var(list_to_free);
		/* free(c.name); // i think this is wrong? */
		if (handle_stack_trace(source))
		{
			free(lex.buffer);
			return false;
		}

	}
	free(lex.buffer);
	return true;
}
