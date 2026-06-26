/* Statistiques partagees du shell.
 * Toutes les mises a jour passent par stats_mutex pour eviter les courses.
 */
#include "dash.h"

#include <stdio.h>

void	stats_add_command(void)
{
	pthread_mutex_lock(&g_shell.stats_mutex);
	g_shell.stats.total_commands++;
	pthread_mutex_unlock(&g_shell.stats_mutex);
}

/* Enregistre un processus lance et son temps d'execution, puis recalcule
 * le temps moyen. Section critique : plusieurs threads paralleles peuvent
 * terminer en meme temps. */
void	stats_add_process(double exec_time)
{
	pthread_mutex_lock(&g_shell.stats_mutex);
	g_shell.stats.total_processes++;
	g_shell.sum_exec_time += exec_time;
	g_shell.stats.average_exec_time
		= g_shell.sum_exec_time / g_shell.stats.total_processes;
	pthread_mutex_unlock(&g_shell.stats_mutex);
}

void	stats_print(void)
{
	pthread_mutex_lock(&g_shell.stats_mutex);
	printf("Commandes exécutées : %d\n", g_shell.stats.total_commands);
	printf("Processus lancés : %d\n", g_shell.stats.total_processes);
	printf("Temps moyen : %.2f s\n", g_shell.stats.average_exec_time);
	pthread_mutex_unlock(&g_shell.stats_mutex);
}
