# Validation de la distribution Linux 1.0.0

**Date :** 22 août 2026  
**Artefact :** `AudioABComparator-1.0.0-linux-x86_64.AppImage`  
**Architecture :** x86_64  
**Taille :** 53 373 432 octets  
**SHA-256 :** `78ab74717bde2dcd525d34dd46e86b7f1fa06d9417bb1ceefd2f68f86d2ba695`

## Construction

L’AppImage a été construite dans une image Ubuntu 22.04 LTS figée par digest, avec GCC 11.4, CMake 3.31.6, Qt 6.9.3 et les modules Qt Multimedia et Qt Wayland. Les binaires téléchargés de `linuxdeploy` et `linuxdeploy-plugin-qt` sont vérifiés par SHA-256 avant utilisation.

Commande de référence :

```sh
./scripts/linux/build-appimage-container.sh
```

Le manifeste AppStream est d’abord validé localement avec `appstreamcli --no-net`. Le second contrôle réseau de l’outil de sortie est désactivé : il rendrait impossible la première construction d’une version qui ajoute des captures dont les URL GitHub n’existent qu’après publication.

## Tests automatisés

La construction exécute CTest avant tout assemblage. Une sortie PulseAudio virtuelle garantit la présence d’un périphérique audio et FFmpeg génère deux pistes de huit secondes dans chacun des formats WAV, FLAC et MP3.

Résultat : **100 % des tests CTest réussis**, y compris les fixtures WAV, FLAC et MP3. Les scénarios couvrent les formats PCM UInt8, Int16, Int32 et Float en mono et stéréo, les références libebur128, l’annulation des résultats obsolètes, les vumètres A/B appariés et leurs extrema, la décision de lecture native ou convertie, la sélection, la lecture, la boucle, la bascule, les votes et le Blind Test.

Les six catalogues traduits contiennent chacun **150 chaînes terminées et aucune chaîne inachevée**. L’AppImage a passé `--smoke-test` dans les sept langues sous Wayland et XWayland/XCB : anglais, français, allemand, espagnol, portugais brésilien, japonais et chinois simplifié.

## Contrôle du paquet

- validation réussie des fichiers `.desktop` et AppStream ;
- présence de Qt 6.9.3 et des bibliothèques FFmpeg 7.1.1 ;
- présence des plugins Qt Multimedia, XCB, Wayland EGL et Wayland générique ;
- présence vérifiée de `wayland-graphics-integration-client/libqt-plugin-wayland-egl.so` et du thème de plateforme `xdgdesktopportal` ;
- présence de l’icône A/B, de la GPL, des notices de tiers, des textes LGPL et de la licence MIT de libebur128 ;
- empreinte de l’artefact recalculée et identique à `SHA256SUMS-linux`.

## Smoke tests graphiques

Le binaire empaqueté a été lancé dans la session KDE Plasma du poste de validation :

- Wayland natif : fenêtre maintenue cinq secondes sans arrêt inattendu ni erreur QML, de plateforme ou de rendu ;
- XWayland/XCB : fenêtre maintenue cinq secondes dans les mêmes conditions ;
- backend Qt Multimedia FFmpeg 7.1.1 : chargé ;
- version affichée `1.0.0` et sélection explicite des sept langues : vérifiées sous les deux plateformes.

La compilation et les tests automatisés ont été exécutés sur la base Ubuntu 22.04 LTS. Un essai graphique interactif complet dans une session Ubuntu GNOME réelle reste à effectuer ; cette limite est consignée ici.
