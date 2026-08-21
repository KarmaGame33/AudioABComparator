# Résumé de version 0.2.1-beta.3

**Date :** 21 août 2026  
**Version technique :** 0.2.1  
**Version affichée et publiée :** 0.2.1-beta.3  
**Statut :** troisième préversion publique

## Résultat

Cette préversion donne à Audio A/B Comparator une identité visuelle cohérente dans l’application, sous KDE/Wayland et dans l’exécutable Windows. Le moteur audio, les formats acceptés, le fonctionnement local et l’absence de télémétrie restent inchangés. Le SVN maître reste canonique et GitHub reste le miroir public officiel.

## Contenu de la version

- nouvelle icône A/B aux couleurs turquoise et violette des deux pistes ;
- ressource PNG embarquée dans l’application ;
- icône Windows multi-résolution intégrée à `ab-compare.exe` ;
- icône SVG et fichier `.desktop` installables sous Linux ;
- identifiant de bureau unique évitant le remplacement par l’icône audio générique de KDE ;
- AppImage Linux x86_64 construite de manière reproductible sous Ubuntu 22.04 LTS avec Qt 6.9.3 ;
- métadonnées AppStream, licences et notices intégrées à l’AppImage ;
- capture et GIF publics reconstruits depuis le binaire Windows final.

## Validation

La compilation Release Linux, CTest et le smoke test QML ont réussi. La construction AppImage exécute les scénarios automatisés avec des pistes WAV, FLAC et MP3 et une sortie PulseAudio virtuelle. Le paquet final a passé les validations AppStream et desktop, le contrôle des dépendances ELF ainsi que les smoke tests KDE sous Wayland natif et XWayland. Sous Windows 10, la reconstruction MSVC/Qt 6.9.3, les 14 scénarios CTest, le smoke test du paquet portable, l’extraction de l’icône de l’EXE et l’intégrité du ZIP ont été validés. La compatibilité annoncée reste **Windows 10 et Windows 11 x64** pour le ZIP et **Linux x86_64** pour l’AppImage.

## Artefact Windows

```text
AudioABComparator-0.2.1-beta.3-windows-x86_64.zip
SHA-256 : 93f3f8a262826ee41cddb8aef208a3dbc3b3a75fc123c6ff7374df0ce6dfbec4
Taille  : 54 207 516 octets
```

Le ZIP contient l’exécutable portable, ses DLL Qt/FFmpeg et VC143, les plugins, QML, le README, la GPL, les notices et les textes LGPL. Il reste distribué uniquement par GitHub Releases.

## Artefact Linux

```text
AudioABComparator-0.2.1-beta.3-linux-x86_64.AppImage
SHA-256 : 13d58f6a9149239f24fa502d0722e559d45260181f6917a614cac9fba2812f60
Taille  : 53 209 592 octets
```

L’AppImage contient Qt 6.9.3, le backend FFmpeg 7.1.1, les plugins XCB et Wayland, les modules QML, l’icône A/B, les métadonnées AppStream, la GPL, les notices de tiers et les textes LGPL. Elle est distribuée uniquement par GitHub Releases.

## Limites

- bêta non signée : SmartScreen peut afficher un avertissement de réputation ;
- l’essai interactif complet sous Ubuntu GNOME reste à effectuer ;
- macOS, installeur et signature Authenticode restent hors périmètre ;
- les retours passent uniquement par les Issues GitHub.

La stratégie interne complète reste décrite dans `docs/strategie_diffusion_github.md`, exclu du miroir GitHub.
