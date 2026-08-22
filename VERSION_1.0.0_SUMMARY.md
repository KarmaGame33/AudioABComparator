# Résumé de version 1.0.0

**Date :** 22 août 2026  
**Version technique, affichée et publiée :** 1.0.0  
**Statut :** première version stable, Windows et Linux

## Résultat

Audio A/B Comparator atteint le périmètre fonctionnel prévu pour l’application. La version 1.0.0 réunit les parcours **Express**, **Blind Test** et **Analyse** dans une application locale, sans compte ni télémétrie.

L’écran Analyse mesure séparément A et B sur le fichier entier ou la sélection commune. Il conserve le PCM natif pour les mesures statiques, affiche simultanément les vumètres temps réel des deux pistes et détaille toute conversion nécessaire à la lecture. L’application n’applique ni normalisation, ni égalisation, ni compression, ni limitation.

## Contenu de la version

- comparaison A/B instantanée, timeline et sélection communes, boucle, raccourcis et votes ;
- sessions Blind Test avec tirage contraint, scores séparés et révélation finale ;
- Sample Peak, True Peak, LUFS-I, LRA, RMS, facteur de crête et décalage DC pour le fichier ou la sélection ;
- vumètres A/B simultanés Sample Peak, True Peak, RMS, LUFS-M et LUFS-S avec repères minimum et maximum ;
- décodage PCM natif, lecture directe lorsque le format commun est accepté et détail des conversions sinon ;
- sélecteur de fichiers système sous Linux avec repli portable Qt ;
- interface complète en sept langues et thèmes clair et sombre ;
- ZIP portable Windows 10/11 x64 et AppImage Linux x86_64.

## Périmètre assumé

Les analyses statiques et temps réel prennent en charge les pistes mono et stéréo. Les fichiers multicanaux restent chargeables, mais leur analyse n’est pas prise en charge. La lecture native décrit le PCM transmis à Qt et ne promet pas un flux bit-perfect après le mixeur du système d’exploitation.

## Politique de version

À partir de cette version, les publications utilisent directement le format `majeure.mineure.correctif`, sans suffixe bêta ou RC. Les anciennes préversions restent dans l’historique sans définir le statut actuel de l’application.

## Validation de publication

- build Release Linux direct et AppImage reproductible ;
- build natif Windows 10 x64 et ZIP portable ;
- CTest avec signaux synthétiques et fixtures WAV, FLAC et MP3 ;
- contrôles QML, traductions et smoke tests dans les sept langues ;
- contrôle des licences, dépendances, manifestes SHA-256 et paquets téléchargés après publication.

## Limites de distribution

- aucun paquet macOS ;
- paquets non signés : Windows SmartScreen peut afficher un avertissement ;
- aucune promesse de compatibilité avec toutes les variantes de codecs acceptées par FFmpeg.
