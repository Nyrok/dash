/* Point d'entrée de la suite de tests : initialise l'état partagé puis exécute
 * une suite par module. Compile avec toutes les sources sauf main.c. */
#include "dash.h"
#include "minunit.h"
#include "tests.h"

#include <stdio.h>
#include <string.h>

int		tests_run = 0;
t_shell	g_shell;

static void	setup(void)
{
	memset(&g_shell, 0, sizeof(g_shell));
	pthread_mutex_init(&g_shell.stats_mutex, NULL);
	pthread_mutex_init(&g_shell.history_mutex, NULL);
	pthread_mutex_init(&g_shell.queue_mutex, NULL);
	pthread_mutex_init(&g_shell.log_mutex, NULL);
	pthread_cond_init(&g_shell.queue_not_empty, NULL);
	pool_init();
}

int	main(void)
{
	const char	*(*suites[])(void) = {
		run_parser_tests, run_path_tests, run_stats_tests,
		run_history_tests, run_builtins_tests, run_pool_tests,
		run_log_tests
	};
	const char	*result;
	size_t		i;

	setup();
	result = NULL;
	i = 0;
	while (i < sizeof(suites) / sizeof(suites[0]) && result == NULL)
		result = suites[i++]();
	if (result != NULL)
		printf("ECHEC : %s\n", result);
	else
		printf("Tous les tests passent.\n");
	printf("Tests executes : %d\n", tests_run);
	return (result != NULL);
}
