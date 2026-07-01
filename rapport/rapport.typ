// ──────────────────────────────────────────────
//  rapport.typ - Projet Systemes d'exploitation
//  dash++ (Dauphine Advanced Shell)
// ──────────────────────────────────────────────

#import "@preview/fletcher:0.5.8" as fletcher: diagram, node, edge

#set document(title: "Projet Systèmes d'exploitation - dash++", author: "Hamza Konte")
#set page(paper: "a4", margin: (x: 2cm, y: 2cm), numbering: "1")
#set text(font: "Helvetica Neue", size: 9pt, lang: "fr")
#set par(justify: true, leading: 0.6em)
#set heading(numbering: "1.1")
#show heading: set text(fill: rgb("#003366"))
#show figure.caption: emph
#show raw.where(block: false): r => text(fill: rgb("#7a1f1f"), r)
#show heading.where(level: 1): it => {
  block(fill: rgb("#003366"), inset: 4pt, width: 100%, text(fill: white, it))
}

#let bleu = rgb("#003366")

#grid(
  columns: (1fr, 1fr, 1fr),
  gutter: 12pt,
  align(top + left, image("Dauphine_logo.jpg", width: 3cm)),
  align(top + center)[
    #text(size: 18pt, weight: "bold", fill: bleu)[dash++]

    #v(0.3em)
    #text(size: 10pt)[Projet de Systèmes d'exploitation]

    #v(0.2em)
    #text(size: 8pt, style: "italic")[
      Hamza Konte, Salim Bekri, Yanis Tounsi \
      #text(size: 6.5pt, fill: gray.darken(50%))[Université Paris-Dauphine PSL] \
      Juillet 2026
    ]
  ],
  align(top + right)[
    #text(size: 12pt, weight: "medium")[L3 MIAGE APP]
  ],
)

#v(0.5em)
#line(length: 100%, stroke: 1pt + bleu)
#v(0.6em)

= Introduction

`dash++` (Dauphine Advanced Shell) est un interpréteur de commandes écrit en C.
Au comportement classique d'un shell (lire une ligne, créer un processus
enfant, exécuter la commande, attendre sa fin) il ajoute une dimension
concurrente : plusieurs threads coopèrent autour de structures partagées pour
tenir un historique, produire des statistiques et superviser l'activité en
temps réel.

Le programme lit les commandes avec `getline`, résout les exécutables via un
chemin de recherche, et lance chaque commande externe par `fork` puis `execv`.
Un thread d'historique, un thread de monitoring et un pool de workers entourent
le thread principal. La vraie difficulté est le partage sûr de la mémoire entre
ces fils d'exécution, et c'est ce que détaille ce rapport.

= Choix techniques

== Le langage C et ses appels système

Le sujet impose le C et la bibliothèque standard uniquement. Ce cadre est en
réalité l'intérêt du projet : on manipule directement les appels système Unix
(`fork`, `execv`, `waitpid`, `dup2`, `access`, `chdir`) et les primitives
POSIX de threads, sans couche d'abstraction#footnote[Pour l'API de gestion des
processus et des threads, nous nous sommes appuyés sur Arpaci-Dusseau, R. & A.,
_Operating Systems: Three Easy Pieces_, chapitres sur les processus (5), les
threads (26) et les verrous (28).]. Le revers est un besoin constant
de vigilance sur la mémoire et les codes de retour. Deux habitudes, héritées de
nos projets précédents, ont servi de filet de sécurité : des commits fréquents
et atomiques pour isoler vite un bug, et un passage systématique sous Valgrind
(`--leak-check=full` pour les fuites, `--tool=helgrind` pour les accès
concurrents).

== Une architecture multithread

Le programme s'organise autour d'une unique instance d'état global, `g_shell`
(type `t_shell`), sur laquelle travaillent le thread principal, deux threads de
service (historique, monitoring) et un pool de workers.

#figure(
  table(
    columns: (auto, 1fr),
    align: (left, left),
    stroke: 0.5pt + bleu,
    table.header([*Thread*], [*Responsabilité*]),
    [Principal], [Prompt `dash> `, lecture (`getline`), analyse, exécution des
      commandes intégrées, dépôt des commandes dans les files (producteur).],
    [Historique], [Retire les commandes de la file (rôle de consommateur), les
      enregistre et sauvegarde périodiquement dans un fichier.],
    [Monitoring], [Affiche toutes les 5 secondes un résumé de l'activité
      (commandes, processus, temps moyen d'exécution).],
    [Workers ($times 4$)], [Pool de threads (bonus) : exécutent les commandes
      externes (`fork`/`execv`/`waitpid`) que le principal leur soumet.],
  ),
  caption: [Les threads et leurs rôles],
) <threads>

Le schéma suivant montre comment ces threads gravitent autour de l'état
partagé. Chaque flèche porte le mutex qui protège l'accès correspondant.

