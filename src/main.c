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
	for (int i = 1; i < argc; i++)
	{
		if (!parse_arg(&conf, argv[i]))
			display_help(conf.mode, 1);
	}
	da_append(conf.flags, '\0');

	if (strchr(conf.flags.items, 'h'))
		display_help(conf.mode, 0);

	printf("%s -%c%s\n", argv[0], conf.mode, conf.flags.items);

	switch (conf.mode)
	{
	case 'S': return handle_sync(conf);
	case 'Q': return handle_query(conf);
	case 'T': return handle_temp(conf);
	case 'C': return handle_container(conf);
	case 'P': return handle_push(conf);
	default: puts("unimplemented mode"); return 1;
	}
}
