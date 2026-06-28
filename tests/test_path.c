/* Tests unitaires du module path.c */
#include "dash.h"
#include "minunit.h"
#include "tests.h"

#include <stdlib.h>
#include <string.h>

static const char	*test_path_init(void)
{
	path_init();
	mu_assert("path_init: un repertoire", g_shell.path_count == 1);
	mu_assert("path_init: /bin", strcmp(g_shell.path[0], "/bin") == 0);
	mu_assert("path_init: NULL-termine", g_shell.path[1] == NULL);
	path_free();
	mu_assert("path_free: liste NULL", g_shell.path == NULL);
	return (0);
}

static const char	*test_path_resolve(void)
{
	char	*full;

	path_init();
	full = path_resolve("ls");
	mu_assert("resolve: ls introuvable dans /bin", full != NULL);
	mu_assert("resolve: chemin absolu", strcmp(full, "/bin/ls") == 0);
	free(full);
	full = path_resolve("binaire_inexistant_9999");
	mu_assert("resolve: faux positif", full == NULL);
	path_free();
	return (0);
}

static const char	*test_path_set(void)
{
	char	*dirs[2] = {"/bin", "/usr/bin"};
	char	*full;

	path_init();
	path_set(dirs, 2);
	mu_assert("path_set: deux repertoires", g_shell.path_count == 2);
	mu_assert("path_set: dir1", strcmp(g_shell.path[1], "/usr/bin") == 0);
	full = path_resolve("ls");
	mu_assert("path_set: ls resolu", full != NULL);
	free(full);

	path_set(dirs, 0);
	mu_assert("path_set: chemin vide", g_shell.path_count == 0);
	full = path_resolve("ls");
	mu_assert("path vide: aucune resolution", full == NULL);
	path_free();
	return (0);
}

const char	*run_path_tests(void)
{
	mu_run_test(test_path_init);
	mu_run_test(test_path_resolve);
	mu_run_test(test_path_set);
	return (0);
}
