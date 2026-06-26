/* Bonus : journalisation concurrente dans shell.log.
 * Chaque ligne comporte un horodatage, l'identifiant du thread et la
 * commande. L'acces au fichier est protege par log_mutex.
 */
#include "dash.h"

#include <pthread.h>
#include <stdio.h>
#include <time.h>

void	log_command(const char *cmd)
{
	FILE		*f;
	time_t		now;
	struct tm	tmv;
	char		stamp[32];

	pthread_mutex_lock(&g_shell.log_mutex);
	f = fopen(LOG_FILE, "a");
	if (f != NULL)
	{
		now = time(NULL);
		localtime_r(&now, &tmv);
		strftime(stamp, sizeof(stamp), "%Y-%m-%d %H:%M:%S", &tmv);
		fprintf(f, "[%s] tid=%lu %s\n", stamp,
			(unsigned long)pthread_self(), cmd);
		if (fclose(f) != 0)
			print_error();
	}
	pthread_mutex_unlock(&g_shell.log_mutex);
}
