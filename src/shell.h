#ifndef SHELL_H_
#define SHELL_H_

#include <unistd.h>
#include <sys/wait.h>

#include "da.h"

typedef struct
{
	DA(char*) argv;
} command;

static inline command _create_cmd_(const char* const argv[])
{
	command cmd = {0};
	da_construct(cmd.argv, 10);
	for (int i = 0; argv[i]; i++)
		da_append(cmd.argv, (char*)argv[i]);
	da_append(cmd.argv, (char*)0);
	return cmd;
}

#define create_cmd(...) _create_cmd_((const char* const[]){ __VA_ARGS__, NULL})

#define cmd_delete(cmd) da_delete((cmd).argv)

static inline pid_t run_cmd_async(command cmd)
{
	pid_t pid = fork();
	char* name = cmd.argv.items[0];
	if (pid == 0)
	{
		execvp(name, cmd.argv.items);
		printf("failed to execvp '%s': %s\n", name, strerror(errno));
		return -1;
	}
	else if (pid == -1)
	{
		printf("failed to fork: %s\n", strerror(errno));
		return -1;
	}
	return pid;
}

static inline int run_cmd(command cmd)
{
	pid_t pid = run_cmd_async(cmd);

	int status, exit_code;
	pid_t err = waitpid(pid, &status, 0);
	if (err != pid)
	{
		printf("failed to waitpid: %s\n", strerror(errno));
		return 1;
	}
	if (WIFEXITED(status))
		exit_code = WEXITSTATUS(status);
	else
		exit_code = -1;

	da_delete(cmd.argv);
	return exit_code;
}

typedef DA(char) _da_char_;

static inline int run_cmd_da(command cmd, _da_char_* out)
{
	int pipefd[2];
	if (pipe(pipefd) == -1)
	{
		printf("failed to create pipe: %s", strerror(errno));
		return -1;
	}
	char* name = cmd.argv.items[0];

	pid_t pid = fork();
	if(pid == 0)
	{
		close(pipefd[0]);
		dup2(pipefd[1], STDOUT_FILENO);
		close(pipefd[1]);

		execvp(name, cmd.argv.items);
		printf("failed to execvp '%s': %s\n", name, strerror(errno));
		return -1;
	}
	else if (pid == -1)
	{
		printf("failed to fork: %s\n", strerror(errno));
		return -1;
	}
	close(pipefd[1]);

	if (!out->items) da_construct(*out, 128);
	char buf[256];
	ssize_t n;
	while ((n = read(pipefd[0], buf, sizeof(buf))) > 0)
	{
		for (int i = 0; i < n; i++) da_append(*out, buf[i]);
	}
	close(pipefd[0]);

	int status;
	waitpid(pid, &status, 0);

	da_append(*out, 0);

	int exit_code;
	if (WIFEXITED(status))
		exit_code = WEXITSTATUS(status);
	else
		exit_code = -1;

	return exit_code;
}

#endif
