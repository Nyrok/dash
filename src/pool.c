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
