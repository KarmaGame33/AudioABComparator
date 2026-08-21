# Résumé de version 0.2.1-beta.4

**Date :** 21 août 2026  
**Version technique :** 0.2.1  
**Version affichée et publiée :** 0.2.1-beta.4  
**Statut :** quatrième préversion publique

## Résultat

Cette préversion rend Audio A/B Comparator disponible dans sept langues : anglais, français, allemand, espagnol, portugais brésilien, japonais et chinois simplifié. L’application détecte la langue du système au premier lancement et permet de la changer immédiatement dans les paramètres, sans redémarrage. Le fonctionnement local, l’absence de télémétrie et le moteur audio restent inchangés.

## Contenu de la version

- six catalogues Qt Linguist embarqués, chacun avec 99 chaînes terminées et aucune chaîne inachevée ;
- choix persistant de la langue dans les paramètres ;
- détection initiale de la langue du système ;
- retraduction immédiate de l’interface QML et des messages d’état ou d’erreur audio ;
- option `--language` pour sélectionner explicitement une langue au lancement ;
- ZIP portable Windows 10/11 x64 et AppImage Linux x86_64 reconstruits avec Qt 6.9.3.

## Validation

Sous Linux, la construction reproductible Ubuntu 22.04, CTest, le contrôle AppStream, l’audit des dépendances et les smoke tests des sept langues ont réussi. Un vrai lancement de fenêtre sous KDE/Wayland a été maintenu huit secondes sans erreur de rendu. Sous Windows 10 22H2, la reconstruction MSVC/Qt 6.9.3 et les 14 scénarios automatisés ont réussi avec une sortie Remote Audio active et les paires WAV, FLAC et MP3. Le ZIP extrait a également passé les smoke tests des sept langues.

La compatibilité annoncée reste **Windows 10 et Windows 11 x64** pour le ZIP et **Linux x86_64** pour l’AppImage.

## Artefact Windows

```text
AudioABComparator-0.2.1-beta.4-windows-x86_64.zip
SHA-256 : 6a1e7cfe9087e9b243ed26b101ecbad8f25a583faac7f738298b4bf32b6e794d
Taille  : 54 228 821 octets
```

## Artefact Linux

```text
AudioABComparator-0.2.1-beta.4-linux-x86_64.AppImage
SHA-256 : b52afd9ff763caf715ef4e0a2ccf384ef917e5db3653e06dd91e0f309d9dd08c
Taille  : 53 262 840 octets
```

Les deux binaires sont distribués uniquement dans GitHub Releases, avec un fichier `SHA256SUMS` commun. Le dépôt GitHub fournit le code source correspondant.

## Limites

- bêta non signée : SmartScreen peut afficher un avertissement de réputation ;
- les nouvelles traductions doivent encore bénéficier de relectures par des locuteurs natifs ;
- l’essai interactif complet sous Ubuntu GNOME reste à effectuer ;
- macOS, installeur et signature Authenticode restent hors périmètre ;
- les retours passent par les Issues GitHub.
