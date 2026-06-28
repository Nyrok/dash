/* Tests unitaires du module history.c (file + historique partagé) */
#include "dash.h"
#include "minunit.h"
#include "tests.h"

#include <string.h>

static const char	*test_queue_push_pop(void)
{
	char	out[HISTORY_LINE];

	memset(&g_shell.queue, 0, sizeof(g_shell.queue));
	queue_push("ls");
	queue_push("pwd");
	mu_assert("queue: taille 2", g_shell.queue.size == 2);
	mu_assert("queue: pop 1", queue_pop(out) == 1);
	mu_assert("queue: FIFO ls", strcmp(out, "ls") == 0);
	mu_assert("queue: pop 2", queue_pop(out) == 1);
	mu_assert("queue: FIFO pwd", strcmp(out, "pwd") == 0);
	mu_assert("queue: vide", g_shell.queue.size == 0);
	return (0);
}

static const char	*test_queue_closed(void)
{
	char	out[HISTORY_LINE];

	memset(&g_shell.queue, 0, sizeof(g_shell.queue));
	g_shell.queue.closed = 1;
	mu_assert("queue: fermee et vide -> 0", queue_pop(out) == 0);
	return (0);
}

static const char	*test_history_add(void)
{
	memset(&g_shell.hist, 0, sizeof(g_shell.hist));
	history_add("ls");
	history_add("pwd");
	mu_assert("hist: count 2", g_shell.hist.count == 2);
	mu_assert("hist: entree 0", strcmp(g_shell.hist.history[0], "ls") == 0);
	mu_assert("hist: entree 1", strcmp(g_shell.hist.history[1], "pwd") == 0);
	history_print(); /* couvre l'affichage */
	return (0);
}

const char	*run_history_tests(void)
{
	mu_run_test(test_queue_push_pop);
	mu_run_test(test_queue_closed);
	mu_run_test(test_history_add);
	return (0);
}
