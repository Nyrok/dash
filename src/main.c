/* dash++ - Dauphine Advanced Shell
 * Boucle interactive principale. Lance les threads historique et moniteur,
 * lit les commandes via getline(), les execute, puis termine proprement.
 */
#include "dash.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

t_shell	g_shell;

static void	shell_init(void)
{
	int	ok;

	memset(&g_shell, 0, sizeof(g_shell));
	g_shell.running = 1;
	path_init();
	pool_init();
	ok = pthread_mutex_init(&g_shell.stats_mutex, NULL) == 0
		&& pthread_mutex_init(&g_shell.history_mutex, NULL) == 0
		&& pthread_mutex_init(&g_shell.queue_mutex, NULL) == 0
		&& pthread_mutex_init(&g_shell.log_mutex, NULL) == 0
		&& pthread_cond_init(&g_shell.queue_not_empty, NULL) == 0;
	if (!ok)
	{
		print_error();
		exit(1);
	}
}

static void	shell_destroy(void)
{
	path_free();
	pool_destroy();
	pthread_mutex_destroy(&g_shell.stats_mutex);
	pthread_mutex_destroy(&g_shell.history_mutex);
	pthread_mutex_destroy(&g_shell.queue_mutex);
	pthread_mutex_destroy(&g_shell.log_mutex);
	pthread_cond_destroy(&g_shell.queue_not_empty);
}
