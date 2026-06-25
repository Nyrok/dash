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
