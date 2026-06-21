/* Analyse de la ligne : decoupage parallele (&), tokenisation, redirection (<).
 * Les operateurs & et < ne necessitent pas d'espace autour d'eux. */
#include "dash.h"

#include <stdlib.h>
#include <string.h>

char	*trim_spaces(char *str)
{
	char	*end;

	while (*str == ' ' || *str == '\t' || *str == '\n')
		str++;
	if (*str == '\0')
		return (str);
	end = str + strlen(str) - 1;
	while (end > str && (*end == ' ' || *end == '\t' || *end == '\n'))
		end--;
	end[1] = '\0';
	return (str);
}

int	split_parallel(char *line, char **commands, int max)
{
	int		count;
	char	*token;
	char	*trimmed;

	count = 0;
	while ((token = strsep(&line, "&")) != NULL && count < max)
	{
		trimmed = trim_spaces(token);
		if (*trimmed != '\0')
			commands[count++] = trimmed;
	}
	return (count);
}
