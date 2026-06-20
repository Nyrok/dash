/* dash++ - Dauphine Advanced Shell
 * En-tête principal : structures partagees, prototypes et etat global.
 */
#ifndef DASH_H
# define DASH_H

# define _POSIX_C_SOURCE 200809L
# define _DEFAULT_SOURCE
# ifdef __APPLE__
#  define _DARWIN_C_SOURCE
# endif

# include <pthread.h>
# include <stdatomic.h>
# include <stddef.h>

