# Audio A/B Comparator 1.0.0

Audio A/B Comparator 1.0.0 is the first stable release. It completes the intended feature set for focused local A/B listening, blind tests and native-PCM mastering analysis.

## Highlights

- instant A/B switching over one shared timeline and selection, with looping, shortcuts and uninterrupted voting;
- constrained Blind Test sessions with separate scores and a final reveal;
- whole-file and selection measurements for A and B: Sample Peak, True Peak, LUFS-I, LRA, RMS, crest factor and DC offset;
- paired Sample Peak, True Peak, RMS, LUFS-M and LUFS-S live meters with independent minimum and maximum markers;
- native PCM retained for analysis and direct playback when both tracks share a device-supported format;
- explicit sample-rate, channel-layout and sample-format conversion details when direct playback is unavailable;
- complete English, French, German, Spanish, Brazilian Portuguese, Japanese and Simplified Chinese interfaces;
- local-only operation with no account, cloud upload or telemetry.

Analysis supports mono and stereo. Multichannel tracks can be loaded, but their static and live analysis is not supported. No normalization, EQ, compression or limiting is applied.

## Downloads

- Windows 10/11 x64: unsigned portable ZIP;
- Linux x86_64: AppImage tested under Wayland and XWayland/XCB;
- `SHA256SUMS`: checksums for both binaries.

On Windows, extract the complete ZIP before running `ab-compare.exe`. SmartScreen may warn because the executable is not Authenticode-signed. On Linux, make the AppImage executable before launching it.

Please report reproducible problems through [GitHub Issues](https://github.com/KarmaGame33/AudioABComparator/issues).

---

Audio A/B Comparator 1.0.0 est la première version stable. Elle réunit l’ensemble des fonctions prévues pour l’écoute A/B locale, les tests à l’aveugle et l’analyse mastering du PCM natif.

## Points principaux

- bascule A/B instantanée sur une timeline et une sélection communes, avec boucle, raccourcis et votes sans interruption ;
- sessions Blind Test avec tirage contraint, scores séparés et révélation finale ;
- mesures du fichier entier et de la sélection pour A et B : Sample Peak, True Peak, LUFS-I, LRA, RMS, facteur de crête et décalage DC ;
- vumètres A/B simultanés Sample Peak, True Peak, RMS, LUFS-M et LUFS-S avec repères minimum et maximum indépendants ;
- PCM natif conservé pour l’analyse et lecture directe lorsque les deux pistes partagent un format accepté par le périphérique ;
- détail des conversions de fréquence, disposition des canaux et type d’échantillon lorsque la lecture directe est indisponible ;
- interface complète en anglais, français, allemand, espagnol, portugais brésilien, japonais et chinois simplifié ;
- fonctionnement entièrement local, sans compte, cloud ni télémétrie.

L’analyse prend en charge les pistes mono et stéréo. Les pistes multicanales peuvent être chargées, mais leur analyse statique et temps réel n’est pas prise en charge. Aucune normalisation, égalisation, compression ni limitation n’est appliquée.

## Téléchargements

- Windows 10/11 x64 : ZIP portable non signé ;
- Linux x86_64 : AppImage testée sous Wayland et XWayland/XCB ;
- `SHA256SUMS` : empreintes des deux binaires.

Sous Windows, extrayez le ZIP complet avant de lancer `ab-compare.exe`. SmartScreen peut afficher un avertissement car l’exécutable ne possède pas de signature Authenticode. Sous Linux, rendez l’AppImage exécutable avant de la lancer.

Merci de signaler les problèmes reproductibles dans les [Issues GitHub](https://github.com/KarmaGame33/AudioABComparator/issues).
