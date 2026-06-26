/* Thread de monitoring : affiche periodiquement l'activite du shell.
 * Les valeurs proviennent des structures partagees protegees par mutex.
 */
#include "dash.h"

#include <stdio.h>
#include <unistd.h>

/* Boucle du thread moniteur. Toutes les MONITOR_PERIOD secondes, affiche un
 * resume. Le sleep est fractionne pour reagir vite a un exit. */
void	*monitor_thread(void *arg)
{
	int		commands;
	int		processes;
	double	average;
	int		waited;

	(void)arg;
	while (g_shell.running)
	{
		waited = 0;
		while (waited < MONITOR_PERIOD && g_shell.running)
		{
			sleep(1);
			waited++;
		}
		if (!g_shell.running)
			break ;
		pthread_mutex_lock(&g_shell.stats_mutex);
		commands = g_shell.stats.total_commands;
		processes = g_shell.stats.total_processes;
		average = g_shell.stats.average_exec_time;
		pthread_mutex_unlock(&g_shell.stats_mutex);
		printf("[MONITOR]\n");
		printf(" * Commandes exécutées : %d\n", commands);
		printf(" * Processus lancés : %d\n", processes);
		printf(" * Temps moyen d'exécution : %.2f s\n", average);
		fflush(stdout);
	}
	return (NULL);
}
