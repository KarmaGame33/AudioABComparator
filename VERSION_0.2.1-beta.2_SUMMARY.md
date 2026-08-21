# Résumé de version 0.2.1-beta.2

**Date :** 21 août 2026  
**Version technique :** 0.2.1  
**Version affichée et publiée :** 0.2.1-beta.2  
**Statut :** deuxième préversion publique

## Résultat

Cette préversion améliore la lisibilité de l’interface d’Audio A/B Comparator sans changer ses formats de fichiers, son fonctionnement local ni son absence de télémétrie. Le SVN maître reste canonique et GitHub reste le miroir public officiel.

## Contenu de la version

- nouveau style moderne et arrondi pour les boutons ;
- suppression de l’indication d’action transitoire près de Paramètres ;
- message explicite lorsque le Blind Test est demandé sans deux pistes prêtes ;
- remplacement des pistes masqué pendant le Blind Test ;
- réglage Début/Fin placé sur une ligne indépendante entre les deux formes d’onde ;
- zones hors sélection maintenues grisées et déplacement de la tête de lecture conservé sur les formes d’onde ;
- action `Recommencer` unique après révélation, avec remise à zéro des scores, sélection aléatoire et reprise immédiate de la lecture.

## Validation

La compilation Release, CTest, le smoke test QML, le packaging portable et la recette Windows 10 sont consignés dans `docs/validation_windows.md`. La compatibilité annoncée reste **Windows 10 et Windows 11 x64** avec Qt 6.9.3 et le backend FFmpeg fourni par Qt.

## Artefact Windows

```text
AudioABComparator-0.2.1-beta.2-windows-x86_64.zip
SHA-256 : 502d7eec604c0bde39efdf9546892ed94cf7fda7f4103b6df8d038f9d664db2f
```

Le ZIP contient l’exécutable portable, ses DLL Qt/FFmpeg et VC143, les plugins, QML, le README, la GPL, les notices et les textes LGPL. Il reste distribué uniquement par GitHub Releases.

## Limites

- bêta non signée : SmartScreen peut afficher un avertissement de réputation ;
- AppImage, macOS, installeur et signature Authenticode restent hors périmètre ;
- les retours passent uniquement par les Issues GitHub.

La stratégie interne complète reste décrite dans `docs/strategie_diffusion_github.md`, exclu du miroir GitHub.
