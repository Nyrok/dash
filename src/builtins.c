/* Commandes integrees : exit, cd, path, history, stats.
 * Elles ne sont pas executees dans un processus enfant.
 */
#include "dash.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int	is_builtin(const char *cmd)
{
	return (strcmp(cmd, "exit") == 0 || strcmp(cmd, "cd") == 0
		|| strcmp(cmd, "path") == 0 || strcmp(cmd, "history") == 0
		|| strcmp(cmd, "stats") == 0);
}
