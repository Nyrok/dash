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

/* fork + execv du binaire resolu, avec redirection d'entree eventuelle.
 * Dans l'enfant on utilise _exit pour ne pas re-vider les tampons stdio
 * herites du parent (qui provoqueraient des sorties dupliquees). */
static pid_t	spawn_process(char *full, char **argv, const char *infile)
{
	pid_t	pid;
	int		fd;

	pid = fork();
	if (pid < 0)
		return (-1);
	if (pid == 0)
	{
		if (infile != NULL)
		{
			fd = open(infile, O_RDONLY);
			if (fd < 0)
			{
				print_error();
				_exit(1);
			}
			if (dup2(fd, STDIN_FILENO) < 0 || close(fd) < 0)
			{
				print_error();
				_exit(1);
			}
		}
		execv(full, argv);
		print_error();
		_exit(1);
	}
	return (pid);
}
