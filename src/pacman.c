#include "pacman.h"

#include "da.h"
#include "shell.h"

#include <errno.h>
#include <ctype.h>

char* read_entire_file(const char* file)
{
	FILE* fp = fopen(file, "r");

	fseek(fp, 0, SEEK_END);
	size_t sz = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	char* buf = malloc(sz + 1);
	fread(buf, 1, sz, fp);
	buf[sz] = 0;

	return buf;
}

const char* trim_whitespace(const char* str)
{
	for (; *str; str++)
		if (!isspace(*str)) break;
	return str;
}

bool handle_directive(config* conf, const char* line)
{
	printf("fuck u this is a todo, no directives\n");
	exit(1);
	return true;
}

bool parse_config_file_line(config* conf, const char* line)
{
	line = trim_whitespace(line);
	if (!*line) return true;
	if (*line == '!') return handle_directive(conf, line);

	while (*line)
	{
		if (*line == '#') return true;
		const char* eop = line;
		while (!isspace(*++eop)); // TODO: kill gcc devs
		size_t line_size = eop - line;
		char* package = malloc(line_size + 1);
		strncpy(package, line, line_size);
		package[line_size] = 0;
		da_append(conf->config_packages, package);
		line = trim_whitespace(eop);
	}

	return 0;
}

bool parse_config_file(config* conf)
{
	/* char* config = read_entire_file(conf->file); TODO: improve */

	size_t sz = 60;
	char buf[sz];

	FILE* fp = fopen(conf->file, "r");
	if (!fp)
	{
		printf("failed to open file '%s': %s", conf->file, strerror(errno));
		exit(1);
	}

	errno = 0;
	while (fgets(buf, sz, fp))
	{
		parse_config_file_line(conf, buf);
	}
	return true;
}

bool query_system_packages(config* conf)
{
	_da_char_ buffer;
	da_construct(buffer, 128);
	command cmd = create_cmd("sudo", "pacman", "-Qqe");
	int code = run_cmd_da(cmd, &buffer);
	if (code != 0)
	{
		da_delete(buffer);
		cmd_delete(cmd);
		return false;
	}

	char* begin = buffer.items;
	char* end;
	while (true)
	{
		end = strchr(begin, '\n');
		if (end) *end = '\0';
		da_append(conf->installed_packages, strdup(begin));
		if (!end) break;
		if (!*++end) break;
		begin = end + 1;
	}

	da_delete(buffer);
	cmd_delete(cmd);
	return true;
}

void add_a_packages(config* conf)
{

}

void add_r_packages(config* conf)
{

}

int perform_update()
{
	command cmd = create_cmd("sudo", "pacman", "-Syu");
	int exit = run_cmd(cmd);
	cmd_delete(cmd);
	return exit;
}

[[noreturn]] void no_arg(char* flag)
{
	printf("no argument specified for %s\n", flag);
	exit(1);
}

[[noreturn]] void trailing_args(const config* conf, int argc)
{
	printf("trailing arguments: ");
	for (int i = argc; i < conf->args.len; i++)
	{
		printf("%s", conf->args.items[i]);
	}
	puts("");
	exit(1);
}

int handle_sync(config* conf)
{
	bool add = false, remove = false, update = false, orphan = false;
	int argc = 0;
	for (int i = 0; conf->flags.items[i]; i++)
	{
		switch (conf->flags.items[i])
		{
		case 'a':
			add = true;
			break;
		case 'r':
			remove = true;
			break;
		case 'u':
			update = true;
			break;
		case 'o':
			orphan = true;
			break;
		case 'c':
			argc++;
			if (argc > conf->args.len) no_arg("-c");
			conf->file = conf->args.items[argc - 1];
			break;
		default:
			printf("why is there a -%c in -S?\n", conf->flags.items[i]);
			exit(1);
		}
	}
	if (conf->args.len > argc) trailing_args(conf, argc);

	if (!conf->file)
	{
		printf("-c unspecified, setting config to '%s'", DEFAULT_CONFIG);
		conf->file = DEFAULT_CONFIG;
	}

	if (!parse_config_file(conf)) exit(1);
	if (!query_system_packages(conf)) exit(1);

	da_sort_string(conf->config_packages);
	da_sort_string(conf->installed_packages);

	for (int i = 0; i < conf->config_packages.len; i++)
	{
		printf("package: '%s'\n", conf->config_packages.items[i]);
	}

	for (int i = 0; i < conf->installed_packages.len; i++)
	{
		/* printf("package: '%s'\n", conf->installed_packages.items[i]); */
	}

	if (!add && !remove && !update && !orphan)
		add = remove = true;

	if (add) add_a_packages(conf);
	if (remove) add_r_packages(conf);
	/* if (orphan) add_o_packages(conf); // TODO */

	if (update) perform_update();
	for (int i = 0; i < conf->config_packages.len; i++)
		free(conf->config_packages.items[i]);
	for (int i = 0; i < conf->installed_packages.len; i++)
		free(conf->installed_packages.items[i]);
	return 0;
}

int handle_query(config* conf)
{
	puts("query unimplemented");
	return 0;
}

int handle_temp(config* conf)
{
	puts("temp unimplemented");
	return 0;
}

int handle_container(config* conf)
{
	puts("container unimplemented");
	return 0;
}

int handle_push(config* conf)
{
	puts("push unimplemented");
	return 0;
}

