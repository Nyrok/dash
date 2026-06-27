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
