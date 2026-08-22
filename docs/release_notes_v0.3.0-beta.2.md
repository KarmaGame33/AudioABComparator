# Audio A/B Comparator 0.3.0-beta.2

This development beta refines the native-PCM Analysis dashboard introduced in 0.3.0-beta.1.

## Changes since 0.3.0-beta.1

- Linux direct builds and AppImages prefer the system file chooser through the desktop portal, with the portable Qt dialog retained as a fallback;
- the Analysis range selector now uses the shorter `All | Selection` labels and consistently colors the active choice green;
- start and end handles retain their mouse drag inside the Analysis scroll view;
- range values are previewed while dragging and selection analysis starts only after release;
- the playback-time badge now sits above the playhead in a dedicated lane and no longer covers the waveform.

The native PCM analysis scope remains mono and stereo. No normalization, EQ, compression or limiting is applied.

0.3.0-beta.2 has been built and tested under Linux from the direct build tree. Native Windows 10 validation and distributable Linux and Windows artifacts are still required before publication.

---

Cette bêta de développement affine l’écran Analyse sur PCM natif introduit en 0.3.0-beta.1.

## Changements depuis 0.3.0-beta.1

- les builds Linux directs et les AppImages préfèrent le sélecteur de fichiers système via le portail de bureau, avec conservation du dialogue Qt portable comme repli ;
- le sélecteur de portée devient `Tout | Sélection` et réserve systématiquement le vert au choix actif ;
- les poignées Début et Fin conservent le glisser de la souris dans la zone défilante de l’écran Analyse ;
- la plage est prévisualisée pendant le déplacement et son analyse ne démarre qu’au relâchement ;
- l’indicateur temporel est placé au-dessus du curseur dans une zone dédiée et ne masque plus la forme d’onde.

L’analyse PCM native reste limitée au mono et à la stéréo. Aucune normalisation, égalisation, compression ni limitation n’est appliquée.

0.3.0-beta.2 a été compilée et testée sous Linux depuis l’arbre de build direct. La validation Windows 10 native et la production des artefacts Linux et Windows restent nécessaires avant publication.
