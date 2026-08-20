# Validation Windows de la version 0.2.1-beta.1

**Date :** 20 août 2026  
**Plateformes annoncées :** Windows 10 et Windows 11 x64  
**Chaîne de publication :** MSVC 2022, CMake/Ninja, Qt 6.9.3 et FFmpeg 7.1.1 fourni par Qt

## Windows 10 x64 — build de publication

La recette finale a été exécutée dans une VM Windows 10 Professionnel 22H2 x64 (`10.0.19045`) avec une session RDP et un périphérique Remote Audio :

- configuration CMake Release avec MSVC 19.44 : réussie ;
- reconstruction propre Ninja (`--clean-first`) : réussie ;
- CTest avec Qt 6.9.3 : 1 cible, 14 scénarios réussis, aucun échec ni scénario ignoré ;
- smoke test QML hors écran de `ab-compare.exe` : code de sortie 0 ;
- installation CMake et `windeployqt` : réussis ;
- déploiement app-local des DLL Microsoft VC143 : réussi ;
- création du ZIP et de `SHA256SUMS` : réussie.

Les scénarios automatisés couvrent notamment les raccourcis distincts et leur refus de conflit, le thème persistant, les scores, le Blind Test aléatoire contraint, le bip PCM et son volume, la timeline commune, les déplacements, la lecture réelle sur la sortie Windows, pause/arrêt/reprise et absence de reprise après arrêt.

Une recette de formats externe a généré deux tonalités stéréo de huit secondes en WAV, FLAC et MP3. Pour chacune des trois paires, Qt Multimedia/FFmpeg a décodé A puis B, rendu le moteur prêt, démarré la lecture, fait avancer la position, activé la boucle, basculé A/B et mené un Blind Test jusqu'à la révélation.

Une automatisation UI sur le binaire déployé a ensuite :

- chargé les deux WAV par les dialogues natifs Windows ;
- affiché les deux formes d'onde et la sélection commune ;
- lancé la lecture puis basculé vers B ;
- démarré une session Blind Test sans exposer la piste active ;
- capturé les trois états réels utilisés dans le README et le GIF public.

## Windows 11 x64

Le build 0.2.0 immédiatement antérieur, fondé sur la même API audio et la même chaîne Qt 6.9.3/MSVC 2022, avait réussi sous Windows 11 x64 : compilation, CTest, chargement QML, installation et déploiement Qt. La 0.2.1-beta.1 ne change pas l'API applicative ni le moteur audio ; elle change la version visible, les tests de parcours séquentiel, la documentation, les licences et le packaging public.

La compatibilité annoncée couvre donc Windows 10 et Windows 11 x64. Une nouvelle validation Windows 11 complète sera requise si le moteur audio, Qt, FFmpeg, MSVC ou le contenu du bundle change.

## Contrôle du paquet

Le ZIP final porte le nom `AudioABComparator-0.2.1-beta.1-windows-x86_64.zip` et son empreinte SHA-256 est `9067da5ca8df6fcc0457e1626d1310de402572c3e1391a0acbb4c7eab72dcdb8`. Les contrôles indépendants ont confirmé :

- `ab-compare.exe` à la racine du dossier versionné ;
- `README.txt`, `LICENSE`, `THIRD_PARTY_NOTICES.md` et `licenses/` présents ;
- DLL Qt 6.9.3, FFmpeg 7.1.1, VC143 et plugins `platforms/`, `multimedia/`, `qml/` présents ;
- aucun PDB, build, cache, chemin de poste ou secret ;
- aucun `vc_redist.x64.exe` ou installeur ;
- empreinte recalculée identique à `SHA256SUMS` ;
- lancement portable sans dépendre de la chaîne de compilation.

## Limites de la validation

L'automatisation prouve le décodage, l'alimentation de la sortie Windows, l'avancement, les états UI et les transitions fonctionnelles. Elle ne remplace pas une écoute humaine pour juger l'absence absolue de clic, le niveau subjectif du bip ou l'ergonomie sur tous les écrans. Ces points restent à surveiller pendant la bêta par Issues GitHub.

Le paquet n'est pas signé Authenticode. SmartScreen peut avertir pour un exécutable récent sans réputation ; l'utilisateur doit vérifier la provenance GitHub et l'empreinte SHA-256.
