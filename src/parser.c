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
