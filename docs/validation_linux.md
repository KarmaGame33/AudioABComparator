# Validation de la distribution Linux 0.2.1-beta.4

**Date :** 21 août 2026  
**Artefact :** `AudioABComparator-0.2.1-beta.4-linux-x86_64.AppImage`  
**Architecture :** x86_64  
**Taille :** 53 262 840 octets  
**SHA-256 :** `b52afd9ff763caf715ef4e0a2ccf384ef917e5db3653e06dd91e0f309d9dd08c`

## Construction

L’AppImage a été construite dans une image Ubuntu 22.04 LTS figée par digest, avec GCC 11.4, CMake 3.31.6, Qt 6.9.3 et les modules Qt Multimedia et Qt Wayland. Les binaires téléchargés de `linuxdeploy` et `linuxdeploy-plugin-qt` sont vérifiés par SHA-256 avant utilisation.

Commande de référence :

```sh
./scripts/linux/build-appimage-container.sh
```

## Tests automatisés

La construction exécute CTest avant tout assemblage. Une sortie PulseAudio virtuelle garantit la présence d’un périphérique audio et FFmpeg génère deux pistes de huit secondes dans chacun des formats WAV, FLAC et MP3.

Résultat : **100 % des tests CTest réussis**. Les scénarios couvrent notamment le décodage des deux pistes, la sélection d’au moins cinq secondes, la lecture, la boucle, la bascule A/B, les votes, le Blind Test, la révélation, le redémarrage de session, les raccourcis et le bip de transition.

Les six catalogues traduits contiennent chacun **99 chaînes terminées et aucune chaîne inachevée**. Le binaire AppImage a également passé `--smoke-test` dans les sept langues : anglais, français, allemand, espagnol, portugais brésilien, japonais et chinois simplifié.

## Contrôle du paquet

- validation réussie des fichiers `.desktop` et AppStream ;
- aucune dépendance ELF manquante ;
- présence de Qt 6.9.3 et des bibliothèques FFmpeg 7.1.1 ;
- présence des plugins Qt Multimedia, XCB, Wayland EGL et Wayland générique ;
- présence vérifiée de `wayland-graphics-integration-client/libqt-plugin-wayland-egl.so` ;
- présence de l’icône A/B, de la GPL, des notices de tiers et des textes LGPL ;
- absence de chemin utilisateur local, de chemin Windows de build et de jeton GitHub dans le paquet ;
- empreinte de l’artefact recalculée et identique à `SHA256SUMS`.

## Smoke tests graphiques

Le binaire empaqueté a été lancé dans la session KDE Plasma du poste de validation :

- Wayland natif : fenêtre maintenue huit secondes sans arrêt inattendu ni erreur OpenGL/RHI ;
- identifiant de fenêtre `io.github.KarmaGame33.AudioABComparator` et icône A/B : présents ;
- backend Qt Multimedia FFmpeg 7.1.1 : chargé ;
- sélection explicite des sept langues en ligne de commande : réussie.

La compilation et les tests automatisés ont été exécutés sur la base Ubuntu 22.04 LTS. Un essai graphique interactif complet dans une session Ubuntu GNOME réelle reste à effectuer ; cette limite est indiquée dans les notes de la bêta.
