# Audio A/B Comparator 0.2.1-beta.3

This beta gives Audio A/B Comparator its own visual identity on the desktop. The audio engine and listening workflows are unchanged from beta.2.

## What’s new

- a dedicated A/B icon using the turquoise and violet colours of the two tracks;
- the icon is embedded in the application window and the Windows executable at multiple resolutions;
- Linux desktop metadata and a scalable icon are installed for application menus and task bars;
- KDE/Wayland now resolves the branded icon through a unique application identifier instead of falling back to a generic audio symbol.

## Windows download

The portable ZIP supports **Windows 10 and Windows 11 x64**. Verify it with the attached `SHA256SUMS`, extract the complete folder and run `ab-compare.exe`. No installer or administrator rights are required.

This beta is unsigned. Microsoft Defender SmartScreen may warn about a new executable with no established reputation. Verify that the download comes from this official Release and that its SHA-256 checksum matches before choosing to run it.

Known beta limits: no signed installer, no AppImage yet, no macOS build, and no guarantee that every codec variant supported by FFmpeg has been exercised. Please report reproducible problems through [GitHub Issues](https://github.com/KarmaGame33/AudioABComparator/issues).

---

Cette bêta donne à Audio A/B Comparator sa propre identité visuelle sur le bureau. Le moteur audio et les parcours d’écoute restent inchangés depuis la beta.2.

## Nouveautés

- icône A/B dédiée reprenant les couleurs turquoise et violette des deux pistes ;
- icône intégrée à la fenêtre de l’application et à l’exécutable Windows en plusieurs résolutions ;
- métadonnées de bureau Linux et icône vectorielle installées pour les menus d’applications et les barres des tâches ;
- sous KDE/Wayland, un identifiant unique permet désormais d’afficher l’icône A/B au lieu d’un symbole audio générique.

Le ZIP portable prend en charge **Windows 10 et Windows 11 x64**. Vérifiez-le avec `SHA256SUMS`, extrayez le dossier complet, puis lancez `ab-compare.exe`. Aucun installeur ni droit administrateur n’est nécessaire.

Cette bêta n’est pas signée : Microsoft Defender SmartScreen peut signaler un exécutable récent sans réputation établie. Vérifiez la provenance GitHub et l’empreinte SHA-256 avant de l’exécuter.

Limites connues : pas d’installeur signé, pas encore d’AppImage, pas de build macOS et couverture non exhaustive de toutes les variantes de codecs. Signalez les problèmes reproductibles dans les [Issues GitHub](https://github.com/KarmaGame33/AudioABComparator/issues).
