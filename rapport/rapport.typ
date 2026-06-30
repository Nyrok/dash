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

