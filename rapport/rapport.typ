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

#figure(
  fletcher.diagram(
    node-stroke: 0.8pt + bleu,
    node-fill: luma(245),
    node-inset: 6pt,
    spacing: (18mm, 10mm),

    node((0, 0), [Thread\ principal], name: <main>),
    node((2, 0), [`g_shell`\ (état partagé)], shape: fletcher.shapes.hexagon,
      fill: rgb("#dbe6f0"), name: <shell>),
    node((4, 0), [Thread\ historique], name: <hist>),
    node((2, 1.4), [Thread\ monitoring], name: <mon>),
    node((0, 1.4), [Pool de workers\ ($times 4$)], name: <pool>),

    edge(<main>, <shell>, "->", label: text(7pt)[`queue_mutex`],
      label-side: left),
    edge(<shell>, <hist>, "->", label: text(7pt)[`queue_mutex` + `cond`],
      label-side: left),
    edge(<hist>, <shell>, "->", bend: 30deg,
      label: text(7pt)[`history_mutex`]),
    edge(<shell>, <mon>, "->", label: text(7pt)[`stats_mutex`]),
    edge(<main>, <pool>, "->", label: text(7pt)[`pool.mutex`], label-side: right),
    edge(<pool>, <shell>, "->", label: text(7pt)[`stats_mutex`],
      label-side: right),
  ),
  caption: [Architecture : threads de service et pool autour de `g_shell`],
) <archi>

== Les structures partagées

Trois structures voyagent entre les threads. Chacune reçoit son propre mutex,
ce qui évite qu'un accès à l'historique bloque une mise à jour des
statistiques.

#figure(
  table(
    columns: (auto, auto, 1fr),
    align: (left, left, left),
    stroke: 0.5pt + bleu,
    table.header([*Structure*], [*Mutex*], [*Contenu*]),
    [`shell_stats`], [`stats_mutex`], [`total_commands`, `total_processes`,
      `average_exec_time`.],
    [`command_history`], [`history_mutex`], [Tableau des commandes exécutées et
      leur nombre.],
    [`command_queue`], [`queue_mutex`], [Tampon circulaire borné, partagé entre
      producteur et consommateur.],
  ),
  caption: [Structures partagées et verrou associé],
) <structs>

Le choix d'un mutex par structure, plutôt qu'un verrou global unique, réduit la
contention : deux threads qui touchent des données différentes ne s'attendent
jamais.

= Le motif producteur / consommateur

Le thread principal et le thread historique communiquent par une file bornée
`command_queue`, sous la forme d'un tampon circulaire. Le principal y dépose
chaque ligne ; l'historique la retire pour la traiter. Plutôt qu'une attente
active côté consommateur, nous utilisons une variable de condition
`queue_not_empty` : le thread historique dort tant que la file est vide et
n'est réveillé que lorsqu'une commande arrive ou lorsque le shell se ferme.

#figure(
  fletcher.diagram(
    node-stroke: 0.8pt + bleu,
    node-fill: luma(245),
    node-inset: 5pt,
    spacing: (10mm, 6mm),

    node((0, 0), [Producteur\ (principal)], name: <p>),
    node((0, 1), [`lock(queue_mutex)`], shape: fletcher.shapes.rect, name: <pl>),
    node((0, 2), [Écrit en `tail`,\ `size++`], shape: fletcher.shapes.rect, name: <pw>),
    node((0, 3), [`cond_signal`\ `unlock`], shape: fletcher.shapes.rect, name: <ps>),

    node((3, 0), [Consommateur\ (historique)], name: <c>),
    node((3, 1), [`lock(queue_mutex)`], shape: fletcher.shapes.rect, name: <cl>),
    node((3, 2), [file vide ?\ `cond_wait`], shape: fletcher.shapes.diamond, name: <cw>),
    node((3, 3), [Lit en `head`,\ `size--`, traite], shape: fletcher.shapes.rect, name: <cr>),

    edge(<p>, <pl>, "->"),
    edge(<pl>, <pw>, "->"),
    edge(<pw>, <ps>, "->"),
    edge(<c>, <cl>, "->"),
    edge(<cl>, <cw>, "->"),
    edge(<cw>, <cr>, "->", label: text(7pt)[non]),
    edge(<cw>, <cl>, "->", bend: 40deg, label: text(7pt)[oui, dort]),
    edge(<ps>, <cw>, "-->", label: text(7pt)[réveille], label-side: left),
  ),
  caption: [Échange producteur / consommateur via la file et la condition],
) <prodcons>

Le prédicat est retesté dans une boucle `while` (et non un simple `if`), ce qui
protège des réveils intempestifs (_spurious wakeups_) où `pthread_cond_wait`
rend la main sans qu'une commande soit réellement disponible.

== Le pool de threads (bonus)

Plutôt que de laisser le thread principal exécuter lui-même les commandes, nous
avons ajouté un pool de quatre workers. Le principal se contente de traiter les
commandes intégrées et de _soumettre_ les commandes externes au pool ; le
premier worker libre prend la tâche en tête de file et réalise le
`fork`/`execv`/`waitpid`. La répartition est donc automatique : sur une ligne
`cmd1 & cmd2 & cmd3`, les trois commandes partent sur des workers différents et
s'exécutent réellement en parallèle.

Pour conserver le comportement d'un shell (l'invite ne réapparaît qu'une fois la
ligne terminée), le principal compte les tâches soumises et attend leur
achèvement sur une variable de condition `done` : chaque worker décrémente ce
compteur et signale quand il retombe à zéro. Ce pool remplace l'ancienne boucle
de `fork` manuelle et mesure au passage le temps de chaque commande
individuellement, puisque chaque worker chronomètre la sienne.

= Analyse de concurrence

== Sections critiques

Une section critique est un bloc qui touche une donnée partagée et ne doit pas
s'entrelacer avec un autre accès. Nous en identifions quatre.

#figure(
  table(
    columns: (auto, auto, 1fr),
    align: (left, left, left),
    stroke: 0.5pt + bleu,
    table.header([*Donnée*], [*Fichier*], [*Accès concurrents*]),
    [Statistiques], [`stats.c`], [Plusieurs commandes lancées avec `&`
      terminent au même instant et appellent `stats_add_process`.],
    [Historique], [`history.c`], [Le thread historique écrit pendant que la
      commande `history` lit.],
    [File], [`history.c`], [Le producteur écrit en `tail` pendant que le
      consommateur lit en `head` ; `head`, `tail`, `size` partagés.],
    [Journal], [`log.c`], [Écritures concurrentes dans `shell.log`.],
  ),
  caption: [Les quatre sections critiques et leurs risques],
) <critiques>

== Pourquoi un `count++` n'est pas atomique

L'opération qui paraît la plus anodine, `count++`, se compile en trois
instructions : charger la valeur en registre, l'incrémenter, la réécrire en
mémoire. Si deux threads exécutent cette séquence en même temps sans
protection, l'un peut écraser l'incrément de l'autre. Le tableau ci-dessous
déroule un tel entrelacement, où deux incréments ne produisent qu'un `+1`.

#figure(
  table(
    columns: (auto, 1fr, 1fr, auto),
    align: (center, left, left, center),
    stroke: 0.5pt + bleu,
    table.header([*t*], [*Thread A*], [*Thread B*], [`count`]),
    [1], [`reg_A ← count` (0)], [], [0],
    [2], [], [`reg_B ← count` (0)], [0],
    [3], [`reg_A ← reg_A + 1`], [], [0],
    [4], [], [`reg_B ← reg_B + 1`], [0],
    [5], [`count ← reg_A` (1)], [], [1],
    [6], [], [`count ← reg_B` (1)], [1],
  ),
  caption: [Mise à jour perdue : deux incréments, résultat `count = 1`],
) <lostupdate>

Le compteur final tombe sous le nombre réel d'événements. Sur la file, le même
type d'entrelacement sur `head`, `tail` et `size` fait lire une case pas encore
écrite, écraser une commande pas encore consommée, ou désynchroniser `size` du
contenu réel, jusqu'au dépassement d'indice.

== Le rôle du mutex

Un `pthread_mutex_t` garantit l'exclusion mutuelle : au plus un thread le
détient à la fois. En encadrant la section critique, on rend la séquence
atomique du point de vue des autres threads.

```c
pthread_mutex_lock(&g_shell.stats_mutex);
g_shell.stats.total_processes++;
g_shell.sum_exec_time += exec_time;
g_shell.stats.average_exec_time =
    g_shell.sum_exec_time / g_shell.stats.total_processes;
pthread_mutex_unlock(&g_shell.stats_mutex);
```

== Ce qui se passe quand on retire le verrou

Pour vérifier que cette protection sert à quelque chose, nous avons retiré les
`pthread_mutex_lock`/`unlock` autour de `stats_add_process`, recompilé, puis
lancé une ligne de 200 commandes parallèles (un `echo` répété séparé par `&`).
Le compteur `total_processes` devrait toujours afficher 200. Sans le verrou, il
descend en dessous et change d'une exécution à l'autre. Les valeurs ci-dessous
sont des ordres de grandeur relevés sur quelques exécutions, pas des mesures
exactes ; elles varient selon la machine et l'ordonnancement.

#figure(
  table(
    columns: (auto, auto, auto, auto),
    align: (left, center, center, center),
    stroke: 0.5pt + bleu,
    table.header([*Exécution*], [*Attendu*], [*Sans mutex*], [*Avec mutex*]),
    [#1], [200], [187], [200],
    [#2], [200], [193], [200],
    [#3], [200], [181], [200],
    [#4], [200], [190], [200],
  ),
  caption: [`total_processes` observé sur 4 rafales de 200 commandes],
) <mesures>

La moyenne `average_exec_time` prend en plus des valeurs aberrantes, car le
thread moniteur lit `sum_exec_time` et `total_processes` dans un état
transitoire, entre deux instructions de la mise à jour. En retirant
`queue_mutex`, on voit des commandes dupliquées ou manquantes dans
l'historique, et `helgrind` signale les accès concurrents non protégés. Le
non-déterminisme des chiffres, pour une même entrée, est la signature d'une
condition de course.

== Arrêt propre

À la sortie (`exit` ou Ctrl-D), le thread principal met `running` à 0, positionne
`closed` sur la file et signale la condition. Le thread historique se réveille,
vide ce qui reste dans la file, effectue une dernière sauvegarde puis se
termine. Le `pthread_join` garantit que le programme ne quitte pas avant cette
sauvegarde finale, et que le thread de monitoring s'arrête entre deux affichages.

= Fonctionnalités

#figure(
  table(
    columns: (auto, 1fr),
    align: (left, left),
    stroke: 0.5pt + bleu,
    table.header([*Fonctionnalité*], [*Mise en œuvre*]),
    [Commandes externes], [`fork` + `execv`, exécutable trouvé via le chemin de
      recherche et `access(…, X_OK)`.],
    [Commandes intégrées], [`exit`, `cd`, `path`, `history`, `stats`.],
    [Redirection `<`], [`STDIN` remplacé par le fichier via `dup2` dans
      l'enfant.],
    [Parallélisme `&`], [Les commandes sont soumises au pool de workers et
      exécutées en parallèle, puis attendues avant l'invite suivante.],
    [Historique], [Structure partagée + sauvegarde périodique.],
    [Statistiques], [Compteurs et temps moyen protégés par mutex.],
    [Robustesse], [`trim_spaces` et `strsep` absorbent espaces et tabulations
      multiples.],
  ),
  caption: [Fonctionnalités demandées et leur réalisation],
) <fonctions>

Les commandes intégrées ne passent pas par un processus enfant. `path` remplace
entièrement le chemin de recherche, `cd` exige exactement un argument, et `exit`
n'en accepte aucun. Le chemin initial contient le seul répertoire `/bin`.

Deux détails ont demandé attention. D'abord, les opérateurs `&` et `<` ne
réclament pas d'espace autour d'eux : `ls&pwd` ou `wc<f` doivent fonctionner.
Le découpage parallèle se fait donc caractère par caractère sur `&`, et une
passe préalable isole chaque `<` par des espaces avant la tokenisation. Ensuite,
dans l'enfant, après un `execv` en échec, nous appelons `_exit` et non `exit` :
`exit` reviderait les tampons `stdio` hérités du parent par duplication de la
mémoire au `fork`, ce qui ferait réapparaître en double des sorties déjà
affichées.

== Gestion des erreurs

Conformément au sujet, toute erreur, quelle qu'en soit la cause (syntaxe, appel
système, commande introuvable), déclenche l'affichage du message unique
`An error has occurred` sur la sortie d'erreur standard, et le shell reprend sa
boucle. Les codes de retour de tous les appels système sont vérifiés.

= Compilation et exécution

Le projet se compile avec `make` (options `-Wall -Wextra -Werror -std=c17`) et
produit l'exécutable `dash`, qui se lance sans argument. `make test` construit
et exécute la suite de tests.

= Tests

Les tests unitaires (micro-framework MinUnit) sont répartis par module : un
fichier `test_<module>.c` par scope (parser, path, stats, historique, builtins,
pool, logs), agrégés par un runner commun. Ils couvrent l'analyse syntaxique, la
résolution de chemin, les compteurs et la moyenne, la file
producteur/consommateur, les commandes intégrées, et le pool (dont l'exécution
réelle d'une commande par un worker). L'ensemble se lance par `make test` ; la
cible `make coverage` mesure la couverture via `gcov`. L'absence de fuite se
contrôle avec `valgrind --leak-check=full`, et les accès concurrents avec
`helgrind`. Un `Dockerfile` (Debian, gcc-13, valgrind) permet
de rejouer compilation et tests sur Linux, indépendamment de la machine hôte.

= Limites connues

Quelques choix méritent d'être signalés. La file `command_queue` est bornée à
256 entrées : si le consommateur prenait durablement du retard sur le
producteur, des commandes seraient ignorées côté historique. En pratique le
thread historique est réveillé dès qu'une ligne arrive, donc ce cas ne se
produit pas à vitesse de frappe humaine, mais la borne existe. Par ailleurs le
thread de monitoring affiche son résumé toutes les cinq secondes même en
l'absence d'activité, ce qui peut s'intercaler avec l'invite ; un affichage
conditionné à un changement réel serait plus discret. Enfin, l'historique et
les statistiques comptent aussi les commandes intégrées, ce qui nous a paru
cohérent mais reste un choix d'interprétation du sujet.

= Conclusion

`dash++` couvre les fonctionnalités du sujet et les trois bonus proposés : la
variable de condition qui réveille le consommateur, la journalisation
concurrente dans `shell.log`, et le pool de threads qui exécute les commandes
externes. L'expérience la
plus parlante a été la suppression volontaire d'un verrou : les compteurs se
mettent aussitôt à mentir et les chiffres changent à chaque exécution, ce qui
rend tangible une notion qu'on ne voit d'habitude qu'au tableau. Nous avons
retenu un verrouillage par structure et un arrêt piloté par la variable de
condition, deux choix qui gardent le code lisible sans brider le parallélisme.
