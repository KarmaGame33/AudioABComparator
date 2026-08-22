# Résumé de version 0.3.0-beta.3

**Date :** 22 août 2026  
**Version technique :** 0.3.0  
**Version affichée et publiée :** 0.3.0-beta.3  
**Statut :** troisième préversion 0.3, Windows et Linux

## Résultat

Cette préversion complète la tranche de mesures dynamiques de l’écran **Analyse**. Les PCM de lecture de A et B sont mesurés simultanément sur la même position, y compris lorsque l’une des pistes n’est pas audible. Les valeurs restent séparées : l’application n’applique ni normalisation, ni égalisation, ni compression, ni limitation.

Le tableau de mastering reste à gauche et les mesures temps réel sont regroupées à droite. Chaque vumètre affiche A au-dessus de B dans la couleur de sa piste. Deux repères discrets indiquent les valeurs minimum et maximum atteintes depuis le démarrage de la lecture ; ils survivent à la pause, à la reprise, au déplacement et à la bascule A/B, puis sont remis à zéro par Stop ou un changement de sélection.

## Contenu de la version

- analyses fichier et sélection pour A et B : Sample Peak, True Peak, LUFS-I, LRA, RMS, facteur de crête et décalage DC ;
- vumètres A/B simultanés Sample Peak, True Peak et RMS sur une fenêtre glissante de 400 ms ;
- loudness momentanée LUFS-M sur 400 ms et courte durée LUFS-S sur 3 s pour les deux pistes ;
- repères minimum et maximum indépendants pour chaque piste et chaque vumètre ;
- contrôleur apparié, calculs sérialisés avec Qt Concurrent et aucune analyse dans le callback audio ;
- tableau de mastering et vumètres disposés en deux colonnes sans superposition ;
- PCM natif conservé pour les analyses statiques et conversion de lecture explicitement détaillée ;
- sélecteur de fichiers système sous Linux via le portail de bureau, avec repli Qt portable ;
- interface complète dans sept langues ;
- nouvelles captures de l’écran Analyse dans la présentation publique.

## Validation de publication

- build Release Linux direct et AppImage ;
- CTest avec signaux synthétiques et fixtures WAV, FLAC et MP3 ;
- QML, catalogues de traduction et smoke tests dans les sept langues ;
- build natif Windows 10 x64, paquet ZIP portable et smoke tests ;
- contrôle des licences, dépendances, manifestes SHA-256 et absence d’artefacts de développement ;
- vérification après téléchargement des pièces jointes GitHub Releases.

## Limites connues

- analyses limitées au mono et à la stéréo ;
- corrélation stéréo, historique LUFS, spectre et analyse multicanale ne font pas partie du périmètre retenu ;
- aucune promesse de flux bit-perfect après PipeWire, le moteur audio Windows ou le mixeur système ;
- paquets non signés : Windows SmartScreen peut afficher un avertissement.
