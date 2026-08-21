# Résumé de version 0.2.1-beta.1

**Date :** 20 août 2026  
**Version technique :** 0.2.1  
**Version affichée et publiée :** 0.2.1-beta.1  
**Statut :** première préversion publique

## Résultat

Audio A/B Comparator — KarmaApps par KarmaGame est publié comme logiciel libre sous GPL-3.0-or-later. Le code source correspondant est fourni par le tag GitHub public, tandis que le ZIP Windows est distribué uniquement comme pièce jointe de GitHub Releases.

Positionnement public : **“Free and open-source A/B audio comparator for mixing, mastering and blind listening tests.”**

## Contenu de la version

- version technique portée à `0.2.1` et version visible à `0.2.1-beta.1` ;
- documentation publique anglaise en premier, suivie d’un résumé français ;
- licence GPL-3.0-or-later, changelog et inventaire des dépendances distribuées ;
- scripts reproductibles de compilation, test et packaging Windows ;
- modèles GitHub pour les bugs et demandes de fonctionnalité ;
- captures réelles Windows, GIF de démonstration et aperçu social 1280×640 ;
- instructions immédiates de compilation Linux ;

Cette version ne change aucune API applicative. Elle reprend les fonctionnalités de comparaison A/B, timeline commune, boucle, Blind Test, raccourcis et thèmes présentes sur le tronc, puis ajoute le cadre de licence, de validation et de publication.

## Validation

Sous Windows 10 Professionnel 22H2 x64, avec MSVC 2022, Qt 6.9.3 et le backend FFmpeg 7.1.1 fourni par Qt :

- reconstruction Release propre et CTest réussis ;
- smoke test QML réussi ;
- décodage WAV, FLAC et MP3, lecture, boucle, bascule A/B et Blind Test validés automatiquement ;
- chargement des deux pistes et parcours UI validés dans une session RDP avec Remote Audio ;
- déploiement Qt/FFmpeg et runtime VC143 app-local validé ;
- archive portable testée indépendamment et contrôlée par SHA-256.

La compatibilité annoncée est **Windows 10 et Windows 11 x64**. Le build 0.2.0 immédiatement antérieur avait été validé sous Windows 11 avec la même API audio et la même chaîne Qt/MSVC ; les changements de cette bêta ne modifient pas le moteur audio.

Le détail et les limites de la recette sont consignés dans `docs/validation_windows.md`. L’automatisation ne remplace pas une écoute humaine exhaustive sur toutes les configurations audio.

## Artefact Windows

```text
AudioABComparator-0.2.1-beta.1-windows-x86_64.zip
SHA-256 : 9067da5ca8df6fcc0457e1626d1310de402572c3e1391a0acbb4c7eab72dcdb8
```

Le ZIP contient `ab-compare.exe`, les DLL Qt/FFmpeg et VC143 requises, les plugins `platforms/`, `multimedia/` et `qml/`, `README.txt`, `LICENSE`, `THIRD_PARTY_NOTICES.md` et les textes LGPL. Aucun installeur ni archive binaire n’est committé dans Git.

## Limites connues et suite

- bêta non signée : SmartScreen peut afficher un avertissement de réputation ;
- retours traités uniquement par les Issues GitHub ; les pull requests ne sont pas traitées pour l’instant ;
- AppImage à produire ultérieurement depuis Ubuntu LTS avec Qt 6.9.3, puis à valider sous KDE/Wayland/PipeWire et Ubuntu GNOME ;
- signature Authenticode, installeur, Microsoft Store, macOS et promesse de version stable hors périmètre de ce lancement.

La procédure complète de version, export, publication, communication, suivi et retrait est décrite dans le document interne `docs/strategie_diffusion_github.md`.
