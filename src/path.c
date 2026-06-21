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
