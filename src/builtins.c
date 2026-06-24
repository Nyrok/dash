/* Commandes integrees : exit, cd, path, history, stats.
 * Elles ne sont pas executees dans un processus enfant.
 */
#include "dash.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int	is_builtin(const char *cmd)
{
	return (strcmp(cmd, "exit") == 0 || strcmp(cmd, "cd") == 0
		|| strcmp(cmd, "path") == 0 || strcmp(cmd, "history") == 0
		|| strcmp(cmd, "stats") == 0);
}

/* exit ne prend aucun argument. On ne fait pas exit(0) tout de suite : il faut
 * d'abord reveiller le thread historique pour qu'il vide la file et sauvegarde. */
static int	builtin_exit(int argc)
{
	if (argc != 1)
	{
		print_error();
		return (0);
	}
	pthread_mutex_lock(&g_shell.queue_mutex);
	g_shell.running = 0;
	g_shell.queue.closed = 1;
	pthread_cond_signal(&g_shell.queue_not_empty);
	pthread_mutex_unlock(&g_shell.queue_mutex);
	return (1); /* signale a la boucle principale de terminer */
}

static void	builtin_cd(char **argv, int argc)
{
	if (argc != 2 || chdir(argv[1]) != 0)
		print_error();
}

static void	builtin_path(char **argv, int argc)
{
	path_set(argv + 1, argc - 1);
}
