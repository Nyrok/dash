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

