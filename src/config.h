#ifndef CONFIG_H_
#define CONFIG_H_

#include "da.h"

typedef struct
{
	char mode;
	DA(char) flags;
	DA(const char*) args;
} config;

void display_help(char mode, int exit_code);
bool parse_arg(config* conf, const char* arg);

#define MODES "SQTCP"
#define GLOBAL_FLAGS "h"
#define S_FLAGS "aro"
#define Q_FLAGS "darc"
#define T_FLAGS "rd"
#define C_FLAGS "rd"
#define P_FLAGS ""

#ifdef CONFIG_H_IMPLEMENTATION

void display_help(char mode, int exit_code)
{
	puts("TODO: help");

	exit(exit_code);
}

bool valid_mode(char mode)
{
	if (!mode) return false;
	return strchr(MODES, mode);
}

bool valid_flag(char mode, char flag)
{
	if (strchr(GLOBAL_FLAGS, flag)) return true;
	if (!valid_mode(mode)) return valid_mode(flag);

	switch (mode)
	{
	case 'S': return strchr(S_FLAGS, flag);
	case 'Q': return strchr(Q_FLAGS, flag);
	case 'T': return strchr(T_FLAGS, flag);
	case 'C': return strchr(C_FLAGS, flag);
	case 'P': return strchr(P_FLAGS, flag);
	default: return false;
	}
}

bool parse_arg(config* conf, const char* arg)
{
	if (arg[0] != '-')
	{
		da_append(conf->args, arg);
		return true;
	}
	// TODO: handle cases where the user wants to pass
	// an argument such as file beginning with -
	if (strlen(arg) < 2) return false;

	for(int i = 1; i < strlen(arg); i++)
	{
		if (!valid_flag(conf->mode, arg[i]))
		{
			if (conf->mode)
				printf("invalid flag %c in mode %c\n", arg[i], conf->mode);
			else
				printf("invalid flag/mode %c\n", arg[i]);

			return false;
		}
		if (!conf->mode && valid_mode(arg[i]))
			conf->mode = arg[i];
		else
			da_append(conf->flags, arg[i]);
	}
	return true;
}

#endif

#endif
