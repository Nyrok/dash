/* Tests unitaires du module pool.c (bonus). Le second test exerce réellement
 * un worker : soumission d'une commande, exécution, puis attente. */
#include "dash.h"
#include "minunit.h"
#include "tests.h"

#include <string.h>

static const char	*test_pool_submit(void)
{
	g_shell.pool.head = 0;
	g_shell.pool.tail = 0;
	g_shell.pool.size = 0;
	g_shell.pool.pending = 0;
	pool_submit("ls");
	mu_assert("pool: taille 1", g_shell.pool.size == 1);
	mu_assert("pool: pending 1", g_shell.pool.pending == 1);
	g_shell.pool.size = 0;
	g_shell.pool.pending = 0;
	return (0);
}

static const char	*test_pool_execution(void)
{
	char		*dirs[2] = {"/bin", "/usr/bin"};
	pthread_t	workers[2];
	int			i;

	path_init();
	path_set(dirs, 2);
	memset(&g_shell.stats, 0, sizeof(g_shell.stats));
	g_shell.pool.head = 0;
	g_shell.pool.tail = 0;
	g_shell.pool.size = 0;
	g_shell.pool.pending = 0;
	g_shell.pool.shutdown = 0;
	i = 0;
	while (i < 2)
		pthread_create(&workers[i++], NULL, worker_thread, NULL);
	pool_submit("true");
	pool_submit("true");
	pool_wait_all();
	mu_assert("pool: 2 process comptes", g_shell.stats.total_processes == 2);
	pthread_mutex_lock(&g_shell.pool.mutex);
	g_shell.pool.shutdown = 1;
	pthread_cond_broadcast(&g_shell.pool.not_empty);
	pthread_mutex_unlock(&g_shell.pool.mutex);
	i = 0;
	while (i < 2)
		pthread_join(workers[i++], NULL);
	path_free();
	return (0);
}

const char	*run_pool_tests(void)
{
	mu_run_test(test_pool_submit);
	mu_run_test(test_pool_execution);
	return (0);
}
