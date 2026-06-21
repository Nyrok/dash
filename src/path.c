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
