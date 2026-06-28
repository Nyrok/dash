/* Tests unitaires du module builtins.c */
#include "dash.h"
#include "minunit.h"
#include "tests.h"

#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static const char	*test_is_builtin(void)
{
	mu_assert("is_builtin: exit", is_builtin("exit"));
	mu_assert("is_builtin: cd", is_builtin("cd"));
	mu_assert("is_builtin: path", is_builtin("path"));
	mu_assert("is_builtin: history", is_builtin("history"));
	mu_assert("is_builtin: stats", is_builtin("stats"));
	mu_assert("is_builtin: ls non integree", !is_builtin("ls"));
	return (0);
}

static const char	*test_builtin_path(void)
{
	char	*argv[3] = {"path", "/bin", "/usr/bin"};

	path_init();
	mu_assert("run_builtin path: pas exit", run_builtin(argv, 3) == 0);
	mu_assert("run_builtin path: 2 repertoires", g_shell.path_count == 2);
	mu_assert("run_builtin path: dir0", strcmp(g_shell.path[0], "/bin") == 0);
	path_free();
	return (0);
}

static const char	*test_builtin_cd(void)
{
	char	*argv[2] = {"cd", "/"};
	char	before[512];
	char	after[512];

	path_init();
	mu_assert("getcwd avant", getcwd(before, sizeof(before)) != NULL);
	mu_assert("run_builtin cd: pas exit", run_builtin(argv, 2) == 0);
	mu_assert("getcwd apres", getcwd(after, sizeof(after)) != NULL);
	mu_assert("cd: repertoire = /", strcmp(after, "/") == 0);
	mu_assert("cd: retour ok", chdir(before) == 0);
	path_free();
	return (0);
}

static const char	*test_builtin_exit(void)
{
	char	*argv[1] = {"exit"};

	g_shell.running = 1;
	mu_assert("run_builtin exit: signale la fin", run_builtin(argv, 1) == 1);
	mu_assert("run_builtin exit: running=0", g_shell.running == 0);
	g_shell.running = 1;
	g_shell.queue.closed = 0;
	return (0);
}

const char	*run_builtins_tests(void)
{
	mu_run_test(test_is_builtin);
	mu_run_test(test_builtin_path);
	mu_run_test(test_builtin_cd);
	mu_run_test(test_builtin_exit);
	return (0);
}
