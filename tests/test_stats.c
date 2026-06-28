/* Tests unitaires du module stats.c */
#include "dash.h"
#include "minunit.h"
#include "tests.h"

#include <string.h>

static const char	*test_stats_commands(void)
{
	memset(&g_shell.stats, 0, sizeof(g_shell.stats));
	stats_add_command();
	stats_add_command();
	stats_add_command();
	mu_assert("stats: 3 commandes", g_shell.stats.total_commands == 3);
	return (0);
}

static const char	*test_stats_average(void)
{
	memset(&g_shell.stats, 0, sizeof(g_shell.stats));
	g_shell.sum_exec_time = 0.0;
	stats_add_process(0.5);
	stats_add_process(1.5);
	mu_assert("stats: 2 process", g_shell.stats.total_processes == 2);
	mu_assert("stats: moyenne 1.0",
		g_shell.stats.average_exec_time > 0.99
		&& g_shell.stats.average_exec_time < 1.01);
	stats_print(); /* couvre l'affichage */
	return (0);
}

const char	*run_stats_tests(void)
{
	mu_run_test(test_stats_commands);
	mu_run_test(test_stats_average);
	return (0);
}
