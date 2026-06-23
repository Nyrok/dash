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

/* Recopie la commande en isolant '<' par des espaces, ce qui permet de le
 * traiter comme un token meme colle a un mot (ex: "wc -l<f"). */
static char	*isolate_redir(const char *command)
{
	char	*out;
	size_t	i;
	size_t	j;

	out = malloc(strlen(command) * 3 + 1);
	if (out == NULL)
		return (NULL);
	i = 0;
	j = 0;
	while (command[i] != '\0')
	{
		if (command[i] == '<')
		{
			out[j++] = ' ';
			out[j++] = '<';
			out[j++] = ' ';
		}
		else
			out[j++] = command[i];
		i++;
	}
	out[j] = '\0';
	return (out);
}

char	**tokenize(char *command, int *argc)
{
	char	**argv;
	char	*token;
	char	*copy;
	char	*cursor;

	/* Borne haute du nombre de tokens : au plus un par caractere. */
	argv = malloc(sizeof(char *) * (strlen(command) + 2));
	copy = isolate_redir(command);
	if (argv == NULL || copy == NULL)
	{
		free(argv);
		free(copy);
		return (NULL);
	}
	*argc = 0;
	cursor = copy;
	while ((token = strsep(&cursor, " \t")) != NULL)
	{
		if (*token == '\0')
			continue;
		argv[*argc] = strdup(token);
		(*argc)++;
	}
	argv[*argc] = NULL;
	free(copy);
	return (argv);
}

/* Retire le motif "< fichier" du tableau. Renvoie -1 si plusieurs '<' ou
 * plusieurs fichiers a droite (erreur de syntaxe). */
int	extract_redirection(char **argv, int *argc, char **infile)
{
	int	i;
	int	found;

	*infile = NULL;
	found = 0;
	i = 0;
	while (argv[i] != NULL)
	{
		if (strcmp(argv[i], "<") == 0)
		{
			if (found || argv[i + 1] == NULL || argv[i + 2] != NULL)
				return (-1);
			*infile = strdup(argv[i + 1]);
			free(argv[i]);
			free(argv[i + 1]);
			argv[i] = NULL;
			*argc = i;
			found = 1;
			return (0);
		}
		i++;
	}
	return (0);
}

void	free_tokens(char **argv)
{
	int	i;

	if (argv == NULL)
		return ;
	i = 0;
	while (argv[i] != NULL)
	{
		free(argv[i]);
		i++;
	}
	free(argv);
}
