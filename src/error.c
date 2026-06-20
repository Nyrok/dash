#include "dash.h"

#include <string.h>
#include <unistd.h>

/* Le sujet impose ce message unique sur stderr pour toute erreur. */
void	print_error(void)
{
	char	error_message[30] = "An error has occurred\n";
	ssize_t	written;

	written = write(STDERR_FILENO, error_message, strlen(error_message));
	(void)written;
}
