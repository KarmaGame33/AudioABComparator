# Audio A/B Comparator 0.3.0-beta.3

This beta publishes the native-PCM Analysis dashboard for Windows and Linux and adds simultaneous live comparison of both tracks.

## Highlights

- whole-file and shared-selection mastering measurements for A and B: Sample Peak, True Peak, LUFS-I, LRA, RMS, crest factor and DC offset;
- paired Sample Peak, True Peak and RMS live meters, with A above B at the same playback position;
- momentary LUFS-M over 400 ms and short-term LUFS-S over 3 seconds for both tracks;
- subtle per-track minimum and maximum markers retained through pause, resume, seeking and A/B switching;
- native decoded PCM retained for static analysis, with direct native playback when both formats match and the output device accepts them;
- explicit sample-rate, channel-layout and sample-format conversion details when native playback is unavailable;
- stable two-column Analysis layout and system file chooser integration on Linux;
- complete English, French, German, Spanish, Brazilian Portuguese, Japanese and Simplified Chinese interfaces.

No normalization, EQ, compression or limiting is applied. Analysis currently supports mono and stereo files; multichannel analysis is explicitly marked unsupported.

## Downloads

- Windows 10/11 x64: portable ZIP, unsigned;
- Linux x86_64: AppImage, tested with Wayland and XWayland/XCB;
- `SHA256SUMS`: checksums for both binaries.

On Windows, extract the complete ZIP before running `ab-compare.exe`. SmartScreen may warn because this beta is not Authenticode-signed. On Linux, make the AppImage executable before launching it.

Please report reproducible problems through [GitHub Issues](https://github.com/KarmaGame33/AudioABComparator/issues).

---

Cette bêta publie l’écran Analyse sur PCM natif pour Windows et Linux et ajoute la comparaison simultanée des mesures temps réel des deux pistes.

## Points principaux

- mesures mastering du fichier entier et de la sélection commune pour A et B : Sample Peak, True Peak, LUFS-I, LRA, RMS, facteur de crête et décalage DC ;
- vumètres Sample Peak, True Peak et RMS appariés, avec A au-dessus de B à la même position de lecture ;
- loudness momentanée LUFS-M sur 400 ms et courte durée LUFS-S sur 3 secondes pour les deux pistes ;
- repères minimum et maximum discrets par piste, conservés pendant pause, reprise, déplacement et bascule A/B ;
- PCM natif décodé conservé pour l’analyse statique et lecture native directe lorsque les deux formats correspondent et sont acceptés par la sortie ;
- détail explicite des conversions de fréquence, disposition des canaux et type d’échantillon lorsque la lecture native est indisponible ;
- écran Analyse stable en deux colonnes et sélecteur de fichiers système sous Linux ;
- interfaces complètes en anglais, français, allemand, espagnol, portugais brésilien, japonais et chinois simplifié.

Aucune normalisation, égalisation, compression ni limitation n’est appliquée. L’analyse prend actuellement en charge les fichiers mono et stéréo ; l’analyse multicanale est explicitement signalée comme non prise en charge.

## Téléchargements

- Windows 10/11 x64 : ZIP portable non signé ;
- Linux x86_64 : AppImage testée sous Wayland et XWayland/XCB ;
- `SHA256SUMS` : empreintes des deux binaires.

Sous Windows, extrayez le ZIP complet avant de lancer `ab-compare.exe`. SmartScreen peut afficher un avertissement car cette bêta ne possède pas de signature Authenticode. Sous Linux, rendez l’AppImage exécutable avant de la lancer.

Merci de signaler les problèmes reproductibles dans les [Issues GitHub](https://github.com/KarmaGame33/AudioABComparator/issues).
