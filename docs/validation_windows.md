# Validation Windows de la version 0.3.0-beta.3

**Date :** 22 août 2026  
**Plateformes annoncées :** Windows 10 et Windows 11 x64  
**Chaîne de publication :** MSVC 2022, CMake/Ninja, Qt 6.9.3 et FFmpeg 7.1.1 fourni par Qt

## Windows 10 x64 — build de publication

La recette finale a été exécutée dans une VM Windows 10 Professionnel 22H2 x64 (`10.0.19045`), dans une session RDP interactive avec sortie Remote Audio :

- configuration CMake Release avec MSVC 19.44 : réussie ;
- reconstruction propre Ninja : réussie ;
- six catalogues traduits de 150 chaînes : aucune chaîne inachevée ;
- CTest avec Qt 6.9.3 : 1 cible, 42 scénarios réussis, aucun échec ni scénario ignoré ;
- décodage et lecture avec sortie audio réelle des paires WAV, FLAC et MP3 : réussis ;
- analyses statiques et temps réel, extrema A/B, annulation des résultats obsolètes et décisions de lecture native ou convertie : réussies ;
- installation CMake, `windeployqt` et déploiement app-local des DLL Microsoft VC143 : réussis ;
- création du ZIP et de `SHA256SUMS` : réussie.

Les scénarios automatisés couvrent notamment UInt8, Int16, Int32 et Float en mono et stéréo, les références libebur128, les raccourcis, les scores, la sélection aléatoire contrainte, le bip PCM, la timeline commune, les déplacements, la lecture réelle, la pause et l’arrêt. Le paquet portable a ensuite passé un vrai processus `--smoke-test` attendu jusqu’à sa fin dans les sept langues : anglais, français, allemand, espagnol, portugais brésilien, japonais et chinois simplifié.

## Windows 11 x64

La chaîne Qt 6.9.3/MSVC 2022 et l’API audio de cette série ont déjà été validées sous Windows 11 x64. Cette version conserve le même format binaire portable et les mêmes dépendances, mais ajoute le moteur d’analyse ; sa recette complète a donc été rejouée sous Windows 10.

La compatibilité annoncée reste donc Windows 10 et Windows 11 x64. La recette complète de cette bêta a été rejouée sous Windows 10 ; elle n’a pas été rejouée sur une seconde machine Windows 11.

## Contrôle du paquet

Le ZIP final est `AudioABComparator-0.3.0-beta.3-windows-x86_64.zip` :

```text
SHA-256 : a70cd009da9aba027b8ce6060ade5a64ef20188c2b90d00a79b4db55415242b3
Taille  : 54 321 062 octets
```

Les contrôles indépendants ont confirmé :

- intégrité ZIP et empreinte recalculée identique à `SHA256SUMS` ;
- 1 421 fichiers, avec `ab-compare.exe` x86-64 à la racine du dossier versionné ;
- `README.txt`, `LICENSE`, `THIRD_PARTY_NOTICES.md`, les textes LGPL et la licence MIT de libebur128 présents ;
- DLL Qt 6.9.3, FFmpeg 7.1.1, VC143 et plugins `platforms/`, `multimedia/` et `qml/` présents ;
- aucun PDB, objet ou bibliothèque de build dans l’archive ;
- smoke tests réussis depuis le dossier portable extrait dans les sept langues.

## Limites de la validation

L’automatisation prouve le décodage, l’alimentation de la sortie Windows, l’avancement, les états UI et les transitions fonctionnelles. Elle ne remplace pas une écoute humaine exhaustive pour juger l’absence absolue de clic ou l’ergonomie sur tous les écrans. Les traductions nouvelles n’ont pas encore toutes été relues par des locuteurs natifs.

Le paquet n’est pas signé Authenticode. SmartScreen peut avertir pour un exécutable récent sans réputation ; l’utilisateur doit vérifier la provenance GitHub et l’empreinte SHA-256.
