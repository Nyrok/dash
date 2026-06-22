/* Execution des commandes : fork + execv, redirection d'entree via dup2,
 * et gestion de l'operateur & pour l'execution parallele.
 */
#include "dash.h"

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <unistd.h>

/* Renvoie l'heure courante en secondes (avec fraction), pour mesurer la
 * duree d'execution des processus. */
static double	now_seconds(void)
{
	struct timeval	tv;

	if (gettimeofday(&tv, NULL) != 0)
		return (0.0);
	return ((double)tv.tv_sec + (double)tv.tv_usec / 1000000.0);
}
