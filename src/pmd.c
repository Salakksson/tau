#include "pmd.h"

#include "da.h"
#include "msg.h"

#include <stdio.h>
#include <string.h>
#include <stdarg.h>

static bool g_thrown = false;

static var clone_var(var v)
{
	switch (v.kind)
	{
	case VAR_ATOM:
		if (v.atom.kind != ATOM_NUMBER)
			v.atom.str = strdup(v.atom.str);
		break;
	case VAR_LIST:
	{
		var_list clone = {0};
		for (size_t i = 0; i < v.list.len; i++)
		{
			var e = clone_var(v.list.arr[i]);
			var_list_append(&clone, e);
		}
		v.list = clone;
		break;
	}
	case VAR_BLOCK:
	{
		var_block clone = {0};

		// TODO: implement shell block copy
		if (v.block.kind != BLOCK_PMD) break;

		for (size_t i = 0; i < v.block.pmd.sts_len; i++)
		{
			statement st = v.block.pmd.sts_arr[i];
			var args = {.kind = VAR_LIST, .list = st.args};
			statement st_clone = {0};
			st_clone.cmd = strdup(st.cmd);
			st_clone.args = clone_var(args).list;
			da_append (
				(void*)&clone.pmd.sts_arr,
				&clone.pmd.sts_len,
				&clone.pmd.sts_cap,
				sizeof(statement),
				&st_clone
			);
		}
		v.block = clone;
		break;
	}
	default:
		break;
	}
	return v;
}

void free_var(var v)
{
	switch (v.kind)
	{
	case VAR_ATOM:
		if (v.atom.kind != ATOM_NUMBER)
		{
			debug("freeing string '%s'", v.atom.str);
			free(v.atom.str);
		}
		break;
	case VAR_LIST:
		for (size_t i = 0; i < v.list.len; i++)
			free_var(v.list.arr[i]);
		debug("freeing list: %p", v.list.arr);
		free(v.list.arr);
		break;
	case VAR_BLOCK:
		for (size_t i = 0; i < v.block.pmd.sts_len; i++)
		{
			statement st = v.block.pmd.sts_arr[i];
			var tmp = {.kind = VAR_LIST, .list = st.args};
			free_var(tmp);
			free(st.cmd);
		}
		debug("freeing block: %p", v.block.pmd.sts_arr);
		free(v.block.pmd.sts_arr);
		break;
	default:
		return;
	}
}

static void define(pmd* p, named_var def)
{
	info("defining var '%s' in scope '%s'", def.name, p->scope);
	def.name = strdup(def.name);
	def.value = clone_var(def.value);
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

// TODO: spit out line number here somehow
void throw(const char* fn_name, const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);

	char* message = vfstring(fmt, args);
	err("%s threw: %s", fn_name, message);
	free(message);

	va_end(args);
	g_thrown = true;
}

bool handle_stack_trace(const char* name)
{
	if (!g_thrown) return false;
	err("trace: '%s'", name);

	return true;
}

// MAIN FUNCTION CALL HANDLE
var pmd_eval_statement(pmd* p, statement st)
{
	/* for (size_t i = 0; i < st.args.len; i++) */
	/*	printf(" %s", var_to_str(p, st.args.arr[i])); */
	/* printf("'\n"); */
	if (!st.cmd) {
		warn("empty statement");
		return (var){0};
	}
	// TODO: actually implement this shit...
	if (!strcmp(st.cmd, "return"))
	{
		if (st.args.len == 1)
			return clone_var(st.args.arr[0]);

		var args = (var) {
			.kind = VAR_LIST,
			.list = st.args,
		};
		return clone_var(args);
	}
	named_var cmd = pmd_get_named_var(p, st.cmd);
	// TODO: check arg count and types against named_var
	// when a synax for that is chosen
	if (!cmd.name)
	{
		throw(p->scope, "undefined variable '%s'", st.cmd);
		return (var){0};
	}

	if (cmd.value.kind != VAR_BLOCK) return cmd.value;

	var_block block = cmd.value.block;

	// evaluate all () expressions
	for (size_t i = 0; i < st.args.len; i++)
	{
		var arg = st.args.arr[i];
		if (arg.kind != VAR_BLOCK) continue;
		if (arg.block.kind != BLOCK_PAREN) continue;
		var eval = pmd_eval(p, arg, cmd.name);
		/* free_var(arg); */
		st.args.arr[i] = eval;
	}

	if (block.kind == BLOCK_C)
		return block.c(p, st.args);

	// TODO: embed shell :/
	/* if (v.block.kind == BLOCK_SHELL) */
	/*	return evaluate_shell_block(p, v.block.shell); */

	// regular {}/() block being evaluated using pmd_eval which creates a scope
	return pmd_eval(p, cmd.value, cmd.name);
}

bool pmd_eval_bool(pmd* p, var cond)
{
	if (cond.kind == VAR_BLOCK) cond = pmd_eval(p, cond, "pmd_eval_bool");
	char* print = var_to_str(p, cond);
	info("eval_bool got: %s", print);
	free(print);
	if (cond.kind == VAR_LIST) return cond.list.len > 0;
	if (cond.kind != VAR_ATOM) return false;

	var_atom atom = cond.atom;

	if (atom.kind == ATOM_VAR)
	{
		named_var v = pmd_get_named_var(p, atom.str);
		return pmd_eval_bool(p, v.value);
	}
	if (atom.kind == ATOM_STRING)
		return atom.str != NULL;
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
		named_var value = pmd_get_named_var(p, v.atom.str);
		var eval = pmd_eval(p, value.value, "var_to_str");
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
	for (size_t i = 0; i < args.len; i++)
	{
		if (i > 0) printf(" ");
		var arg = args.arr[i];
		if (arg.kind == VAR_ATOM && arg.atom.kind == ATOM_STRING)
		{
			printf("%s", arg.atom.str);
			break;
		}
		char* str = var_to_str(p, arg);
		puts(str);
		free(str);
	}
	printf("\n");
	return NULL_VAR;
}

var impl_def(pmd* p, var_list args)
{
	if (args.len < 3)
	{
		throw("def", "not enough args");
		return NULL_VAR;
	}

	bool found_eq = false;
	size_t i;
	named_var def = {0};

	for (i = 0; i < args.len; i++)
	{
		var arg = args.arr[i];
		if (arg.kind != VAR_ATOM)
		{
			// TODO: upgrade logging
			// TODO: also handle this shit better
			throw("def", "undexpected '%s' before =", var_to_str(p, arg));
			return NULL_VAR;

		}

		var_atom atom = arg.atom;

		if (atom.kind != ATOM_VAR)
		{
			throw("def", "unexpected '%s' before =", var_to_str(p, arg));
			return NULL_VAR;

		}

		if (!strcmp(atom.str, "="))
		{
			found_eq = true;
			i++;
			break;
		}
		if (!def.name)
			def.name = atom.str;
		/* else */
			/* var_list_append(&def.args, arg); */
			// TODO: when uder definfed function syntax ...
		continue;

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

	int n_args_left = args.len - i;
	if (n_args_left != 1)
	{
		printf("%d\n", n_args_left);
		for (int j = 0; j < n_args_left; j++)
		{
			printf("%i: %s\n", j, args.arr[i+j].atom.str);
		}
		throw("def", "more than one arg after '=");
		return NULL_VAR;
	}
	def.value = args.arr[i];
	define(p, def);
	return NULL_VAR;
}

var impl_if (pmd* p, var_list args)
{
	// TODO: else
	if (args.len != 2)
	{
		throw("if", "if takes two arguments: `if <cond> <value>`");
		return NULL_VAR;
	}
	var cond = args.arr[0];
	var value = args.arr[1];

	bool eval = pmd_eval_bool(p, cond);
	if (eval)
		return pmd_eval(p, value, "if");
	else
		return NULL_VAR;
}

void def_c_func(pmd* p, const char* name, c_func f)
{
	named_var def = {0};
	def.name = strdup(name);
	def.value.kind = VAR_BLOCK;
	def.value.block.kind = BLOCK_C;
	def.value.block.c = f;
	define(p, def);
}

void def_number(pmd* p, const char* name, int value)
{
	named_var def = {0};
	def.name = strdup(name);
	def.value.kind = VAR_ATOM;
	def.value.atom.kind = ATOM_NUMBER;
	def.value.atom.value = value;
	define(p, def);
}

void def_string(pmd* p, const char* name, const char* str)
{
	named_var def = {0};
	def.name = strdup(name);
	def.value.kind = VAR_ATOM;
	def.value.atom.kind = ATOM_STRING;
	def.value.atom.str = (char*)str;
	define(p, def);
}

bool pmd_init(pmd* p)
{
	p->vars_count = 0;
	p->vars_size = 40;
	p->vars = malloc(sizeof(named_var) * p->vars_size);

	def_c_func(p, "def", impl_def);
	def_c_func(p, "if", impl_if);
	def_c_func(p, "echo", impl_echo);

	def_number(p, "false", 0);
	def_number(p, "true", 1);
	return true;
}

var pmd_eval(pmd* p, var v, const char* scope)
{
	if (v.kind != VAR_BLOCK) return v;

	var last = {0};

	pmd clone = pmd_clone(p, scope);

	for (size_t i = 0; i < v.block.pmd.sts_len; i++)
	{
		statement st = v.block.pmd.sts_arr[i];
		/* info("evaluating statement '%s'", st.cmd); */
		last = pmd_eval_statement(&clone, st);
		// TODO: handle return call somewhere where
		if (handle_stack_trace(clone.scope))
			return (var){0};
	}
	return last;
}

pmd pmd_clone(pmd* p, const char* scope)
{
	debug("cloning pmd to scope %s", scope);
	pmd clone = *p;
	clone.vars = malloc(p->vars_size * sizeof(named_var));
	memcpy(clone.vars, p->vars, p->vars_size * sizeof(named_var));
	clone.scope = strdup(scope);
	return clone;
}

void pmd_free(pmd* p)
{
	warn("TODO: should free pmd");
}

