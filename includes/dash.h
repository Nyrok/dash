/* dash++ - Dauphine Advanced Shell
 * En-tête principal : structures partagees, prototypes et etat global.
 */
#ifndef DASH_H
# define DASH_H

# define _POSIX_C_SOURCE 200809L
# define _DEFAULT_SOURCE
# ifdef __APPLE__
#  define _DARWIN_C_SOURCE
# endif

# include <pthread.h>
# include <stdatomic.h>
# include <stddef.h>

/* Limites du sujet */
# define HISTORY_MAX 1000
# define HISTORY_LINE 256
# define QUEUE_MAX 256
# define WORKER_COUNT 4 /* bonus : nombre de workers du pool de threads */
# define MONITOR_PERIOD 5 /* secondes entre deux affichages du moniteur */
# define HISTORY_SAVE_FILE "history.txt"
# define LOG_FILE "shell.log"

# define PROMPT "dash> "

/* --- Structures partagees demandees par le sujet --- */

typedef struct s_shell_stats
{
	int		total_commands;   /* commandes acceptees par le shell */
	int		total_processes;  /* processus enfants reellement lances */
	double	average_exec_time; /* temps moyen d'execution en secondes */
}	shell_stats;

typedef struct s_command_history
{
	char	history[HISTORY_MAX][HISTORY_LINE];
	int		count;
}	command_history;

/* File producteur/consommateur : le thread principal depose les lignes,
 * le thread historique les consomme. Tampon circulaire borne. */
typedef struct s_command_queue
{
	char	buffer[QUEUE_MAX][HISTORY_LINE];
	int		head; /* index de lecture (consommateur) */
	int		tail; /* index d'ecriture (producteur) */
	int		size; /* nombre d'elements presents */
	int		closed; /* vrai quand le shell se termine */
}	command_queue;

/* Bonus : pool de threads. Le thread principal soumet les commandes externes,
 * les workers les executent (fork/execv/wait). File bornee + compteur de taches
 * en cours pour que le principal attende la fin d'une ligne avant le prompt. */
typedef struct s_exec_task
{
	char	command[HISTORY_LINE];
}	exec_task;

typedef struct s_thread_pool
{
	exec_task		tasks[QUEUE_MAX];
	int				head;
	int				tail;
	int				size;
	int				pending;
	int				shutdown;
	pthread_mutex_t	mutex;
	pthread_cond_t	not_empty;
	pthread_cond_t	done;
}	thread_pool;

/* Etat global partage entre tous les threads. Une seule instance : g_shell. */
typedef struct s_shell
{
	shell_stats		stats;
	command_history	hist;
	command_queue	queue;
	thread_pool		pool;

	char			**path;      /* repertoires de recherche, NULL-termine */
	int				path_count;

	atomic_int		running;     /* drapeau d'arret lu/ecrit par plusieurs threads */
	double			sum_exec_time; /* somme des temps, sert a la moyenne */

	pthread_mutex_t	stats_mutex;
	pthread_mutex_t	history_mutex;
	pthread_mutex_t	queue_mutex;
	pthread_mutex_t	log_mutex;
	pthread_cond_t	queue_not_empty; /* bonus : reveil du consommateur */
}	t_shell;

extern t_shell	g_shell;

/* --- error.c --- */
void	print_error(void);

