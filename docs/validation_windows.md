# Validation Windows de la version 0.2.1-beta.3

**Date :** 21 août 2026  
**Plateformes annoncées :** Windows 10 et Windows 11 x64  
**Chaîne de publication :** MSVC 2022, CMake/Ninja, Qt 6.9.3 et FFmpeg 7.1.1 fourni par Qt

## Windows 10 x64 — build de publication

La recette finale a été exécutée dans une VM Windows 10 Professionnel 22H2 x64 (`10.0.19045`), dans une session RDP interactive avec sortie Remote Audio :

- configuration CMake Release avec MSVC 19.44 : réussie ;
- reconstruction propre Ninja : réussie ;
- CTest avec Qt 6.9.3 : 1 cible, 14 scénarios réussis, aucun échec ni scénario ignoré ;
- smoke test QML hors écran de `ab-compare.exe` : code de sortie 0 ;
- installation CMake et `windeployqt` : réussis ;
- déploiement app-local des DLL Microsoft VC143 : réussi ;
- création du ZIP et de `SHA256SUMS` : réussie.

Les scénarios automatisés couvrent les raccourcis, les scores, la sélection aléatoire contrainte, le bip PCM, la timeline commune, les déplacements, la lecture réelle, la pause et l’arrêt. La recette externe a chargé et décodé des paires WAV, FLAC et MP3 avec zéro test ignoré.

Le changement applicatif de cette bêta est limité à l’identité visuelle. Les contrôles spécifiques ont confirmé :

- une ressource ICO Windows comportant sept tailles, de 16 à 256 pixels ;
- l’icône A/B extraite directement de l’exécutable portable ;
- le démarrage hors écran depuis le dossier final empaqueté ;
- de nouvelles captures réelles montrant l’icône A/B dans la barre de titre Windows.

La recette fonctionnelle complète de la beta.2 — lecture, boucle, bascule A/B, Blind Test et réglage Début/Fin — reste applicable, le moteur audio et les parcours QML n’ayant pas changé dans cette bêta.

## Windows 11 x64

La chaîne Qt 6.9.3/MSVC 2022 et l’API audio de cette série ont déjà été validées sous Windows 11 x64. La beta.3 conserve le même format binaire, les mêmes dépendances, le même backend audio et les mêmes parcours que la beta.2 ; elle ajoute uniquement les ressources d’icône et l’intégration de bureau.

La compatibilité annoncée reste donc Windows 10 et Windows 11 x64. La recette complète de cette bêta a été rejouée sous Windows 10 ; elle n’a pas été rejouée sur une seconde machine Windows 11.

## Contrôle du paquet

Le ZIP final est `AudioABComparator-0.2.1-beta.3-windows-x86_64.zip` :

```text
SHA-256 : 93f3f8a262826ee41cddb8aef208a3dbc3b3a75fc123c6ff7374df0ce6dfbec4
Taille  : 54 207 516 octets
```

Les contrôles indépendants ont confirmé :

- intégrité ZIP sans erreur et empreinte recalculée identique à `SHA256SUMS` ;
- `ab-compare.exe` x86-64 à la racine du dossier versionné ;
- `README.txt`, `LICENSE`, `THIRD_PARTY_NOTICES.md` et `licenses/` présents ;
- DLL Qt 6.9.3, FFmpeg 7.1.1, VC143 et plugins `platforms/`, `multimedia/` et `qml/` présents ;
- aucun PDB, objet, bibliothèque de build, archive imbriquée, chemin privé ou secret ;
- lancement réel depuis le dossier portable produit.

## Limites de la validation

L’automatisation prouve le décodage, l’alimentation de la sortie Windows, l’avancement, les états UI et les transitions fonctionnelles. Elle ne remplace pas une écoute humaine exhaustive pour juger l’absence absolue de clic ou l’ergonomie sur tous les écrans.

Le paquet n’est pas signé Authenticode. SmartScreen peut avertir pour un exécutable récent sans réputation ; l’utilisateur doit vérifier la provenance GitHub et l’empreinte SHA-256.
