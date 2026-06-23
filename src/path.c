/* Gestion du chemin de recherche (search path).
 * Le chemin initial contient uniquement /bin. La commande integree `path`
 * remplace entierement la liste. path_resolve() cherche un executable.
 */
#include "dash.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* Termine le programme si une allocation echoue (etat non recuperable). */
static void	*xalloc(void *ptr)
{
	if (ptr == NULL)
	{
		print_error();
		exit(1);
	}
	return (ptr);
}

void	path_init(void)
{
	g_shell.path = xalloc(malloc(sizeof(char *) * 2));
	g_shell.path[0] = xalloc(strdup("/bin"));
	g_shell.path[1] = NULL;
	g_shell.path_count = 1;
}

/* Remplace entierement le chemin ; une liste vide desactive toute exécution. */
void	path_set(char **dirs, int count)
{
	int	i;

	path_free();
	g_shell.path = xalloc(malloc(sizeof(char *) * (count + 1)));
	i = 0;
	while (i < count)
	{
		g_shell.path[i] = xalloc(strdup(dirs[i]));
		i++;
	}
	g_shell.path[count] = NULL;
	g_shell.path_count = count;
}

/* Renvoie le premier "<dir>/<cmd>" executable (access X_OK), alloue, ou NULL.
 * L'appelant libere la chaine. */
char	*path_resolve(const char *cmd)
{
	int		i;
	char	*full;
	size_t	len;

	i = 0;
	while (i < g_shell.path_count)
	{
		len = strlen(g_shell.path[i]) + strlen(cmd) + 2;
		full = malloc(len);
		if (full == NULL)
			return (NULL);
		snprintf(full, len, "%s/%s", g_shell.path[i], cmd);
		if (access(full, X_OK) == 0)
			return (full);
		free(full);
		i++;
	}
	return (NULL);
}

void	path_free(void)
{
	int	i;

	if (g_shell.path == NULL)
		return ;
	i = 0;
	while (g_shell.path[i] != NULL)
	{
		free(g_shell.path[i]);
		i++;
	}
	free(g_shell.path);
	g_shell.path = NULL;
	g_shell.path_count = 0;
}
