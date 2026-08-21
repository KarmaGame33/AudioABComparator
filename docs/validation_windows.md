# Validation Windows de la version 0.2.1-beta.2

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

Les scénarios automatisés couvrent les raccourcis, les scores, la sélection aléatoire contrainte, le bip PCM, la timeline commune, les déplacements, la lecture réelle, la pause et l’arrêt. Ils vérifient aussi que `Recommencer` réinitialise les compteurs du Blind Test, sélectionne une piste et relance la lecture.

La recette externe a chargé des paires WAV, FLAC et MP3, démarré la lecture, activé la boucle, basculé A/B et mené un Blind Test. Les 14 scénarios sont passés avec zéro test ignoré.

Une automatisation UI sur le binaire portable empaqueté a ensuite :

- chargé les deux WAV par les dialogues natifs Windows ;
- affiché les deux formes d’onde, leur tête de lecture et la ligne Début/Fin indépendante ;
- confirmé les zones hors sélection et les nouveaux boutons ;
- lancé le Blind Test et confirmé la disparition des boutons de remplacement ;
- produit la capture et le GIF réels utilisés dans le README public.

## Windows 11 x64

La chaîne Qt 6.9.3/MSVC 2022 et l’API audio de cette série ont déjà été validées sous Windows 11 x64. La bêta 2 conserve le même format binaire, les mêmes dépendances et le même backend audio ; ses changements portent sur l’interface et l’orchestration du redémarrage d’une session aveugle.

La compatibilité annoncée reste donc Windows 10 et Windows 11 x64. La recette complète de cette bêta a été rejouée sous Windows 10 ; elle n’a pas été rejouée sur une seconde machine Windows 11.

## Contrôle du paquet

Le ZIP final est `AudioABComparator-0.2.1-beta.2-windows-x86_64.zip` :

```text
SHA-256 : 502d7eec604c0bde39efdf9546892ed94cf7fda7f4103b6df8d038f9d664db2f
Taille  : 54 115 492 octets
```

Les contrôles indépendants ont confirmé :

- intégrité ZIP sans erreur et empreinte recalculée identique à `SHA256SUMS` ;
- `ab-compare.exe` x86-64 à la racine du dossier versionné ;
- `README.txt`, `LICENSE`, `THIRD_PARTY_NOTICES.md` et `licenses/` présents ;
- DLL Qt 6.9.3, FFmpeg 7.1.1, VC143 et plugins `platforms/`, `multimedia/` et `qml/` présents ;
- aucun PDB, objet, bibliothèque de build, archive imbriquée, chemin de poste ou secret ;
- lancement réel depuis le dossier portable produit.

## Limites de la validation

L’automatisation prouve le décodage, l’alimentation de la sortie Windows, l’avancement, les états UI et les transitions fonctionnelles. Elle ne remplace pas une écoute humaine exhaustive pour juger l’absence absolue de clic ou l’ergonomie sur tous les écrans.

Le paquet n’est pas signé Authenticode. SmartScreen peut avertir pour un exécutable récent sans réputation ; l’utilisateur doit vérifier la provenance GitHub et l’empreinte SHA-256.
