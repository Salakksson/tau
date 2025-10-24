#include "pmd.h"
#include "msg.h"

#include <string.h>
#include <stdio.h>

#include <sanitizer/lsan_interface.h>

char* get_source_name(int argc, char** argv)
{
	char* source = "";

	for (int i = 1; i < argc; i++)
	{
		char* arg = argv[i];
		bool config = false;

		// search for -c flag
		if (!strcmp(arg, "--config")) config = true;
		if (arg[0] != '-') continue;
		if (arg[1] == '-') continue;

		for (int i = 1; arg[i]; i++)
		{
			if (arg[i] != 'c') continue;
			config = true;
			break;
		}
		if (!config) continue;

		// search for config arg once -c is found
		while (++i < argc)
		{
			char* arg = argv[i];
			if (arg[0] == '-') continue;
			return arg;
		}

		printf("-c with no argument provided, continuing with default config");
		printf("-h exists for those who may need it");
		break;
	}

	return source;
}

int main(int argc, char** argv)
{
	msg_set_level(MSG_DEBUG);
	__lsan_disable();
	/* const char* source_file = get_source_name(argc, argv); */
	pmd p = {0};
	pmd_init(&p);

	var source_tree = create_tree("test.tau");
	bool exit = pmd_eval_bool(&p, source_tree);

	if (!exit)
	{
		warn("testfile failed");
	}

	pmd_free(&p);
	return !exit;
}
