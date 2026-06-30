# dash++ — Dauphine Advanced Shell

Interpréteur de commandes multithread écrit en C dans le cadre du projet de
Systèmes d'exploitation. Il exécute des commandes Unix, gère les processus,
l'exécution concurrente, un historique partagé et affiche des statistiques via
plusieurs threads coopérants.

## Auteurs

- Hamza Konte
- Salim Bekri
- Yanis Tounsi

## Architecture

Le programme s'organise autour de plusieurs threads partageant un état global
protégé par des mutex :

| Thread          | Rôle |
|-----------------|------|
| Principal       | Prompt, lecture, analyse, `fork`/`execv`, producteur de la file |
| Historique      | Consommateur de la file, enregistrement et sauvegarde périodique |
| Monitoring      | Affichage des statistiques toutes les 5 secondes |
| Workers (×4)    | Pool de threads exécutant les commandes externes (bonus) |

Les structures partagées (`shell_stats`, `command_history`, `command_queue`)
sont définies dans `includes/dash.h`. La file suit un schéma
producteur/consommateur avec variable de condition (`pthread_cond_t`).

## Fonctionnalités

- Exécution de commandes externes via `fork` + `execv` avec recherche dans le
  `path` (`access(..., X_OK)`).
- Commandes intégrées : `exit`, `cd`, `path`, `history`, `stats`.
- Redirection d'entrée `<` via `dup2`.
- Exécution parallèle avec l'opérateur `&`.
- Historique partagé et statistiques temps réel.
- Robustesse aux espaces et tabulations multiples.
- Bonus : variable de condition, pool de threads, journalisation concurrente
  dans `shell.log`.

## Compilation

```
make
```

Produit l'exécutable `dash`. Les options `-Wall -Wextra -Werror -std=c17` sont
activées.

## Exécution

```
./dash
dash> ls -la /tmp
dash> wc -l < fichier.txt
dash> sleep 5 & ls & pwd
dash> exit
```

L'interpréteur ne prend aucun argument ; tout argument provoque une erreur.

## Tests

```
make test        # compile et lance toutes les suites
make coverage    # couverture via gcov
```

Les tests unitaires (framework maison MinUnit) sont organisés par module dans
`tests/` : un fichier `test_<module>.c` par scope (parser, path, stats,
historique, builtins, pool, logs), agrégés par `tests/test_main.c`.

## Détection de fuites mémoire

