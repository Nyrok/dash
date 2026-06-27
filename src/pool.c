/* Bonus : pool de threads. Un ensemble fixe de workers exécute les commandes
 * externes soumises par le thread principal. La répartition est automatique :
 * le premier worker libre prend la tâche en tête de file.
 */
#include "dash.h"

#include <string.h>

void	pool_init(void)
{
	pthread_mutex_init(&g_shell.pool.mutex, NULL);
	pthread_cond_init(&g_shell.pool.not_empty, NULL);
	pthread_cond_init(&g_shell.pool.done, NULL);
}

void	pool_destroy(void)
{
	pthread_mutex_destroy(&g_shell.pool.mutex);
	pthread_cond_destroy(&g_shell.pool.not_empty);
	pthread_cond_destroy(&g_shell.pool.done);
}

/* Dépose une commande à exécuter et incrémente le compteur de tâches en cours. */
void	pool_submit(const char *command)
{
	pthread_mutex_lock(&g_shell.pool.mutex);
	strncpy(g_shell.pool.tasks[g_shell.pool.tail].command, command,
		HISTORY_LINE - 1);
	g_shell.pool.tasks[g_shell.pool.tail].command[HISTORY_LINE - 1] = '\0';
	g_shell.pool.tail = (g_shell.pool.tail + 1) % QUEUE_MAX;
	g_shell.pool.size++;
	g_shell.pool.pending++;
	pthread_cond_signal(&g_shell.pool.not_empty);
	pthread_mutex_unlock(&g_shell.pool.mutex);
}

/* Bloque jusqu'à ce que toutes les tâches soumises soient terminées.
 * Sert au thread principal pour n'afficher le prompt qu'après la ligne. */
void	pool_wait_all(void)
{
	pthread_mutex_lock(&g_shell.pool.mutex);
	while (g_shell.pool.pending > 0)
		pthread_cond_wait(&g_shell.pool.done, &g_shell.pool.mutex);
	pthread_mutex_unlock(&g_shell.pool.mutex);
}

/* Boucle d'un worker : attend une tâche, l'exécute hors section critique,
 * puis signale la fin quand plus aucune tâche n'est en cours. */
void	*worker_thread(void *arg)
{
	exec_task	task;

	(void)arg;
	while (1)
	{
		pthread_mutex_lock(&g_shell.pool.mutex);
		while (g_shell.pool.size == 0 && !g_shell.pool.shutdown)
			pthread_cond_wait(&g_shell.pool.not_empty, &g_shell.pool.mutex);
		if (g_shell.pool.size == 0 && g_shell.pool.shutdown)
		{
			pthread_mutex_unlock(&g_shell.pool.mutex);
			break ;
		}
		task = g_shell.pool.tasks[g_shell.pool.head];
		g_shell.pool.head = (g_shell.pool.head + 1) % QUEUE_MAX;
		g_shell.pool.size--;
		pthread_mutex_unlock(&g_shell.pool.mutex);

		run_external_command(task.command);

		pthread_mutex_lock(&g_shell.pool.mutex);
		g_shell.pool.pending--;
		if (g_shell.pool.pending == 0)
			pthread_cond_signal(&g_shell.pool.done);
		pthread_mutex_unlock(&g_shell.pool.mutex);
	}
	return (NULL);
}
