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

/* Exécute une commande externe unique : tokenisation, redirection, résolution
 * du chemin, fork/exec et attente. Appelée par les workers du pool, donc
 * potentiellement par plusieurs threads en parallèle (stats sont protégées). */
void	run_external_command(char *command)
{
	char	**argv;
	char	*infile;
	char	*full;
	int		argc;
	pid_t	pid;
	int		status;
	double	start;

	argv = tokenize(command, &argc);
	if (argv == NULL || argc == 0)
	{
		free_tokens(argv);
		return ;
	}
	if (extract_redirection(argv, &argc, &infile) != 0)
	{
		print_error();
		free_tokens(argv);
		return ;
	}
	full = path_resolve(argv[0]);
	if (full == NULL)
		print_error();
	else
	{
		start = now_seconds();
		pid = spawn_process(full, argv, infile);
		free(full);
		if (pid < 0)
			print_error();
		else if (waitpid(pid, &status, 0) < 0)
			print_error();
		else
			stats_add_process(now_seconds() - start);
	}
	free(infile);
	free_tokens(argv);
}
