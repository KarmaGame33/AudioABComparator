# Audio A/B Comparator 0.2.1-beta.2

This beta refreshes the interface and makes its two core workflows clearer while preserving the local, telemetry-free audio engine.

## What’s new

- modern rounded buttons with clearer hover, pressed and focus states;
- a dedicated start/end control row between the two waveforms, with the excluded portions still shaded on both waveforms;
- waveform dragging remains dedicated to moving the playhead;
- a clear message when Blind Test is requested before both tracks are ready;
- track replacement is hidden during Blind Test;
- after revealing results, a single `Recommencer` action resets the scores, randomly selects a track and immediately resumes playback.

## Windows download

The portable ZIP supports **Windows 10 and Windows 11 x64**. Verify it with the attached `SHA256SUMS`, extract the complete folder and run `ab-compare.exe`. No installer or administrator rights are required.

This beta is unsigned. Microsoft Defender SmartScreen may warn about a new executable with no established reputation. Verify that the download comes from this official Release and that its SHA-256 checksum matches before choosing to run it.

Known beta limits: no signed installer, no AppImage yet, no macOS build, and no guarantee that every codec variant supported by FFmpeg has been exercised. Please report reproducible problems through [GitHub Issues](https://github.com/KarmaGame33/AudioABComparator/issues).

---

Cette bêta modernise l’interface et clarifie ses deux parcours principaux, sans modifier le fonctionnement local et sans télémétrie du moteur audio.

## Nouveautés

- boutons modernes et arrondis avec des états de survol, pression et focus plus lisibles ;
- ligne indépendante entre les formes d’onde pour régler le début et la fin, avec maintien des zones grisées hors sélection sur les deux formes d’onde ;
- déplacement de la tête de lecture toujours effectué directement sur les formes d’onde ;
- message explicite si le Blind Test est demandé avant que les deux pistes soient prêtes ;
- remplacement des pistes masqué pendant le Blind Test ;
- après révélation, une seule action `Recommencer` remet les scores à zéro, tire une piste au hasard et relance immédiatement la lecture.

Le ZIP portable prend en charge **Windows 10 et Windows 11 x64**. Vérifiez-le avec `SHA256SUMS`, extrayez le dossier complet, puis lancez `ab-compare.exe`. Aucun installeur ni droit administrateur n’est nécessaire.

Cette bêta n’est pas signée : Microsoft Defender SmartScreen peut signaler un exécutable récent sans réputation établie. Vérifiez la provenance GitHub et l’empreinte SHA-256 avant de l’exécuter.

Limites connues : pas d’installeur signé, pas encore d’AppImage, pas de build macOS et couverture non exhaustive de toutes les variantes de codecs. Signalez les problèmes reproductibles dans les [Issues GitHub](https://github.com/KarmaGame33/AudioABComparator/issues).
