/* Tests unitaires des modules log.c (bonus) et error.c. On vérifie surtout que
 * ces fonctions s'exécutent sans erreur et écrivent bien leur sortie. */
#include "dash.h"
#include "minunit.h"
#include "tests.h"

#include <stdio.h>

static const char	*test_log_command(void)
{
	FILE	*f;

	remove(LOG_FILE);
	log_command("ls -la");
	f = fopen(LOG_FILE, "r");
	mu_assert("log: fichier non cree", f != NULL);
	if (f != NULL)
		fclose(f);
	remove(LOG_FILE);
	return (0);
}

static const char	*test_error_message(void)
{
	/* Couvre print_error ; le message part sur stderr. */
	print_error();
	return (0);
}

const char	*run_log_tests(void)
{
	mu_run_test(test_log_command);
	mu_run_test(test_error_message);
	return (0);
}
