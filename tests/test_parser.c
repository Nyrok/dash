/* Tests unitaires du module parser.c */
#include "dash.h"
#include "minunit.h"
#include "tests.h"

#include <stdlib.h>
#include <string.h>

static const char	*test_trim(void)
{
	char	buf[64];

	strcpy(buf, "\t  ls -la  \n");
	mu_assert("trim: bords non retires", strcmp(trim_spaces(buf), "ls -la") == 0);
	strcpy(buf, "pwd");
	mu_assert("trim: chaine propre modifiee", strcmp(trim_spaces(buf), "pwd") == 0);
	strcpy(buf, "");
	mu_assert("trim: vide", strcmp(trim_spaces(buf), "") == 0);
	strcpy(buf, " \t \n");
	mu_assert("trim: que des blancs", strcmp(trim_spaces(buf), "") == 0);
	return (0);
}

static const char	*test_tokenize(void)
{
	char	**argv;
	int		argc;

	argv = tokenize("ls   -la\t/tmp", &argc);
	mu_assert("tokenize: mauvais argc", argc == 3);
	mu_assert("tokenize: arg0", strcmp(argv[0], "ls") == 0);
	mu_assert("tokenize: arg2", strcmp(argv[2], "/tmp") == 0);
	mu_assert("tokenize: non NULL-termine", argv[3] == NULL);
	free_tokens(argv);

	argv = tokenize("   ", &argc);
	mu_assert("tokenize: vide -> argc 0", argc == 0);
	mu_assert("tokenize: vide -> NULL", argv[0] == NULL);
	free_tokens(argv);
	return (0);
}

static const char	*test_tokenize_redir(void)
{
	char	**argv;
	int		argc;

	argv = tokenize("wc -l<f", &argc);
	mu_assert("tokenize: < colle mal isole", argc == 4);
	mu_assert("tokenize: token <", strcmp(argv[2], "<") == 0);
	mu_assert("tokenize: fichier apres <", strcmp(argv[3], "f") == 0);
	free_tokens(argv);
	return (0);
}

static const char	*test_split_parallel(void)
{
	char	line[64];
	char	*cmds[8];

	strcpy(line, "sleep 5 & ls & pwd");
	mu_assert("split: mauvais compte", split_parallel(line, cmds, 8) == 3);
	mu_assert("split: cmd0", strcmp(cmds[0], "sleep 5") == 0);
	mu_assert("split: cmd2", strcmp(cmds[2], "pwd") == 0);
	return (0);
}

static const char	*test_split_edge(void)
{
	char	line[32];
	char	*cmds[8];

	strcpy(line, "a&b");
	mu_assert("split: & colle", split_parallel(line, cmds, 8) == 2);
	strcpy(line, "ls &");
	mu_assert("split: & final", split_parallel(line, cmds, 8) == 1);
	strcpy(line, "   ");
	mu_assert("split: ligne vide", split_parallel(line, cmds, 8) == 0);
	strcpy(line, "  &  & ");
	mu_assert("split: que des &", split_parallel(line, cmds, 8) == 0);
	return (0);
}

static const char	*test_redirection(void)
{
	char	**argv;
	char	*infile;
	int		argc;

	argv = tokenize("wc -l < file.txt", &argc);
	mu_assert("redir: ok attendu", extract_redirection(argv, &argc, &infile) == 0);
	mu_assert("redir: fichier", infile && strcmp(infile, "file.txt") == 0);
	mu_assert("redir: argc reduit", argc == 2);
	free(infile);
	free_tokens(argv);
	return (0);
}

static const char	*test_redirection_errors(void)
{
	char	**argv;
	char	*infile;
	int		argc;

	argv = tokenize("cat < a < b", &argc);
	mu_assert("redir: double < non rejetee",
		extract_redirection(argv, &argc, &infile) == -1);
	free_tokens(argv);

	argv = tokenize("cat < a b", &argc);
	mu_assert("redir: token en trop non rejete",
		extract_redirection(argv, &argc, &infile) == -1);
	free_tokens(argv);

	argv = tokenize("cat <", &argc);
	mu_assert("redir: fichier manquant non rejete",
		extract_redirection(argv, &argc, &infile) == -1);
	free_tokens(argv);

	argv = tokenize("wc -l<f", &argc);
	mu_assert("redir: < colle refuse", extract_redirection(argv, &argc, &infile) == 0);
	mu_assert("redir: fichier colle", infile && strcmp(infile, "f") == 0);
	free(infile);
	free_tokens(argv);
	return (0);
}

const char	*run_parser_tests(void)
{
	mu_run_test(test_trim);
	mu_run_test(test_tokenize);
	mu_run_test(test_tokenize_redir);
	mu_run_test(test_split_parallel);
	mu_run_test(test_split_edge);
	mu_run_test(test_redirection);
	mu_run_test(test_redirection_errors);
	return (0);
}
