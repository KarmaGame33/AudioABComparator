# Validation Windows de la version 0.2.1-beta.4

**Date :** 21 août 2026  
**Plateformes annoncées :** Windows 10 et Windows 11 x64  
**Chaîne de publication :** MSVC 2022, CMake/Ninja, Qt 6.9.3 et FFmpeg 7.1.1 fourni par Qt

## Windows 10 x64 — build de publication

La recette finale a été exécutée dans une VM Windows 10 Professionnel 22H2 x64 (`10.0.19045`), dans une session RDP interactive avec sortie Remote Audio :

- configuration CMake Release avec MSVC 19.44 : réussie ;
- reconstruction propre Ninja : réussie ;
- six catalogues traduits de 99 chaînes : aucune chaîne inachevée ;
- CTest avec Qt 6.9.3 : 1 cible, 14 scénarios réussis, aucun échec ni scénario ignoré ;
- décodage et lecture avec sortie audio réelle des paires WAV, FLAC et MP3 : réussis ;
- installation CMake, `windeployqt` et déploiement app-local des DLL Microsoft VC143 : réussis ;
- création du ZIP et de `SHA256SUMS` : réussie.

Les scénarios automatisés couvrent les raccourcis, les scores, la sélection aléatoire contrainte, le bip PCM, la timeline commune, les déplacements, la lecture réelle, la pause et l’arrêt. Le paquet extrait a ensuite passé `--smoke-test` dans les sept langues : anglais, français, allemand, espagnol, portugais brésilien, japonais et chinois simplifié.

## Windows 11 x64

La chaîne Qt 6.9.3/MSVC 2022 et l’API audio de cette série ont déjà été validées sous Windows 11 x64. La beta.4 conserve le même format binaire, les mêmes dépendances et le même moteur audio que la beta.3 ; elle ajoute les traductions et leur sélection à chaud.

La compatibilité annoncée reste donc Windows 10 et Windows 11 x64. La recette complète de cette bêta a été rejouée sous Windows 10 ; elle n’a pas été rejouée sur une seconde machine Windows 11.

## Contrôle du paquet

Le ZIP final est `AudioABComparator-0.2.1-beta.4-windows-x86_64.zip` :

```text
SHA-256 : 6a1e7cfe9087e9b243ed26b101ecbad8f25a583faac7f738298b4bf32b6e794d
Taille  : 54 228 821 octets
```

Les contrôles indépendants ont confirmé :

- intégrité ZIP et empreinte recalculée identique à `SHA256SUMS` ;
- 1 432 entrées, avec `ab-compare.exe` x86-64 à la racine du dossier versionné ;
- `README.txt`, `LICENSE`, `THIRD_PARTY_NOTICES.md` et `licenses/` présents ;
- DLL Qt 6.9.3, FFmpeg 7.1.1, VC143 et plugins `platforms/`, `multimedia/` et `qml/` présents ;
- aucun PDB, objet ou bibliothèque de build dans l’archive ;
- smoke tests réussis depuis le dossier portable extrait dans les sept langues.

## Limites de la validation

L’automatisation prouve le décodage, l’alimentation de la sortie Windows, l’avancement, les états UI et les transitions fonctionnelles. Elle ne remplace pas une écoute humaine exhaustive pour juger l’absence absolue de clic ou l’ergonomie sur tous les écrans. Les traductions nouvelles n’ont pas encore toutes été relues par des locuteurs natifs.

Le paquet n’est pas signé Authenticode. SmartScreen peut avertir pour un exécutable récent sans réputation ; l’utilisateur doit vérifier la provenance GitHub et l’empreinte SHA-256.
