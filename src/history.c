/* Thread historique + file producteur/consommateur.
 *
 * Le thread principal (producteur) depose chaque ligne dans command_queue.
 * Le thread historique (consommateur) attend sur une variable de condition,
 * enregistre les commandes dans la structure partagee et sauvegarde
 * periodiquement le tout dans un fichier.
 */
#include "dash.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>

/* Producteur : ajoute une ligne dans la file bornee, puis reveille le
 * consommateur. Si la file est pleine, la ligne est ignoree (borne du sujet). */
void	queue_push(const char *line)
{
	pthread_mutex_lock(&g_shell.queue_mutex);
	if (g_shell.queue.size < QUEUE_MAX)
	{
		strncpy(g_shell.queue.buffer[g_shell.queue.tail], line,
			HISTORY_LINE - 1);
		g_shell.queue.buffer[g_shell.queue.tail][HISTORY_LINE - 1] = '\0';
		g_shell.queue.tail = (g_shell.queue.tail + 1) % QUEUE_MAX;
		g_shell.queue.size++;
	}
	pthread_cond_signal(&g_shell.queue_not_empty);
	pthread_mutex_unlock(&g_shell.queue_mutex);
}

/* Consommateur : retire une ligne. Attend tant que la file est vide et que
 * le shell tourne. Renvoie 0 quand il n'y a plus rien et que la file est
 * fermee (fin de vie du thread). */
int	queue_pop(char *out)
{
	pthread_mutex_lock(&g_shell.queue_mutex);
	while (g_shell.queue.size == 0 && !g_shell.queue.closed)
		pthread_cond_wait(&g_shell.queue_not_empty, &g_shell.queue_mutex);
	if (g_shell.queue.size == 0 && g_shell.queue.closed)
	{
		pthread_mutex_unlock(&g_shell.queue_mutex);
		return (0);
	}
	strncpy(out, g_shell.queue.buffer[g_shell.queue.head], HISTORY_LINE - 1);
	out[HISTORY_LINE - 1] = '\0';
	g_shell.queue.head = (g_shell.queue.head + 1) % QUEUE_MAX;
	g_shell.queue.size--;
	pthread_mutex_unlock(&g_shell.queue_mutex);
	return (1);
}

void	history_add(const char *line)
{
	pthread_mutex_lock(&g_shell.history_mutex);
	if (g_shell.hist.count < HISTORY_MAX)
	{
		strncpy(g_shell.hist.history[g_shell.hist.count], line,
			HISTORY_LINE - 1);
		g_shell.hist.history[g_shell.hist.count][HISTORY_LINE - 1] = '\0';
		g_shell.hist.count++;
	}
	pthread_mutex_unlock(&g_shell.history_mutex);
}

void	history_print(void)
{
	int	i;

	pthread_mutex_lock(&g_shell.history_mutex);
	i = 0;
	while (i < g_shell.hist.count)
	{
		printf("%d %s\n", i + 1, g_shell.hist.history[i]);
		i++;
	}
	pthread_mutex_unlock(&g_shell.history_mutex);
}
