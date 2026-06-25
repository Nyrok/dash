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
