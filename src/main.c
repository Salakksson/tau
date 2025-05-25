#include <stdio.h>
#include <stdbool.h>

#define CONFIG_H_IMPLEMENTATION
#include "config.h"
#include "pacman.h"

int main(int argc, char** argv)
{
	config conf = {0};
	da_construct(conf.args, 5);
	da_construct(conf.flags, 5);
	da_construct(conf.config_packages, 50);
	da_construct(conf.installed_packages, 50);
	for (int i = 1; i < argc; i++)
	{
		if (!parse_arg(&conf, argv[i]))
			display_help(conf.mode, 1);
	}
	da_append(conf.flags, '\0');

	if (strchr(conf.flags.items, 'h'))
		display_help(conf.mode, 0);

	for (int i = 0; i < conf.args.len; i++) puts(conf.args.items[i]);
	printf("%s -%c%s\n", argv[0], conf.mode, conf.flags.items);

	int exit_code = 1;
	switch (conf.mode)
	{
	case 'S':
		exit_code = handle_sync(&conf);
		break;
	case 'Q':
		exit_code = handle_query(&conf);
		break;
	case 'T':
		exit_code = handle_temp(&conf);
		break;
	case 'C':
		exit_code = handle_container(&conf);
		break;
	case 'P':
		exit_code = handle_push(&conf);
		break;
	default: puts("unimplemented mode");
		exit_code = 1;
		break;
	}

	da_delete(conf.flags);
	da_delete(conf.args);
	da_delete(conf.config_packages);
	da_delete(conf.installed_packages);

	return exit_code;
}
