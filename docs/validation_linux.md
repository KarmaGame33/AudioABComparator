# Validation de la distribution Linux 0.2.1-beta.3

**Date :** 21 août 2026  
**Artefact :** `AudioABComparator-0.2.1-beta.3-linux-x86_64.AppImage`  
**Architecture :** x86_64  
**Taille :** 53 209 592 octets  
**SHA-256 :** `13d58f6a9149239f24fa502d0722e559d45260181f6917a614cac9fba2812f60`

## Construction

L’AppImage a été construite dans une image Ubuntu 22.04 LTS figée par digest, avec GCC 11.4, CMake 3.31.6, Qt 6.9.3 et les modules Qt Multimedia et Qt Wayland. Les binaires téléchargés de `linuxdeploy` et `linuxdeploy-plugin-qt` sont vérifiés par SHA-256 avant utilisation.

Commande de référence :

```sh
./scripts/linux/build-appimage-container.sh
```

## Tests automatisés

La construction exécute CTest avant tout assemblage. Une sortie PulseAudio virtuelle garantit la présence d’un périphérique audio et FFmpeg génère deux pistes de huit secondes dans chacun des formats WAV, FLAC et MP3.

Résultat : **100 % des tests CTest réussis**. Les scénarios couvrent notamment le décodage des deux pistes, la sélection d’au moins cinq secondes, la lecture, la boucle, la bascule A/B, les votes, le Blind Test, la révélation, le redémarrage de session, les raccourcis et le bip de transition.

## Contrôle du paquet

- validation réussie des fichiers `.desktop` et AppStream ;
- extraction réussie des 1 710 fichiers de l’AppImage ;
- aucune dépendance ELF manquante ;
- présence de Qt 6.9.3 et des bibliothèques FFmpeg 7.1.1 ;
- présence des plugins Qt Multimedia, XCB, Wayland EGL et Wayland générique ;
- présence de l’icône A/B, de la GPL, des notices de tiers et des textes LGPL ;
- absence de chemin utilisateur local, de chemin Windows de build et de jeton GitHub dans le paquet.

## Smoke tests graphiques

Le binaire empaqueté a été lancé avec `--smoke-test` dans la session KDE Plasma du poste de validation :

- Wayland natif : réussi ; l’application transmet `io.github.KarmaGame33.AudioABComparator` comme identifiant de fenêtre ;
- XWayland/XCB : réussi ;
- backend Qt Multimedia FFmpeg 7.1.1 chargé : réussi.

La compilation et les tests automatisés ont été exécutés sur la base Ubuntu 22.04 LTS. Un essai graphique interactif complet dans une session Ubuntu GNOME réelle reste à effectuer ; cette limite est indiquée dans les notes de la bêta.
