# Audio A/B Comparator 0.3.0-beta.1

This development beta adds a native-PCM mastering Analysis dashboard while preserving the existing Express and Blind Test workflows.

## What’s new

- whole-file and shared-selection Sample Peak, True Peak, LUFS-I, LRA, RMS, crest factor and DC offset for tracks A and B;
- native decoded PCM retained for exact analysis, with background computation and stale-selection rejection;
- direct native-format playback when both tracks match and the default device accepts the format;
- explicit sample-rate, channel-layout and sample-format conversion details for each track otherwise;
- statically linked libebur128 1.2.6 with its MIT license;
- a translated `Express | Blind Test | Analysis` selector in all seven interface languages.
- the system file chooser through the desktop portal in Linux builds, including direct builds, with the portable Qt dialog retained as a fallback.

This slice supports mono and stereo analysis. Multichannel analysis, real-time meters, LUFS-M/S, correlation and spectrum views remain outside this beta. “Native playback” does not promise bit-perfect output after the operating-system mixer. No normalization, EQ, compression or limiting is applied.

0.3.0-beta.1 has been built and tested under Linux. Native Windows 10 validation and distributable artifacts are still required before publication.

---

Cette bêta de développement ajoute un tableau de bord de mastering sur le PCM natif, sans modifier les workflows Express et Blind Test existants.

## Nouveautés

- Sample Peak, True Peak, LUFS-I, LRA, RMS, facteur de crête et décalage DC pour A et B, sur le fichier entier ou la sélection commune ;
- conservation du PCM natif décodé pour une analyse exacte, calcul en arrière-plan et rejet des résultats de sélection obsolètes ;
- lecture directe du format natif lorsque les deux pistes correspondent et que le périphérique par défaut l’accepte ;
- détail des conversions de fréquence, disposition des canaux et type d’échantillon pour chaque piste dans les autres cas ;
- libebur128 1.2.6 liée statiquement avec sa licence MIT ;
- sélecteur `Express | Blind Test | Analyse` traduit dans les sept langues de l’interface.
- sélecteur de fichiers système via le portail de bureau dans les builds Linux, y compris les builds directs, avec conservation du dialogue Qt portable comme solution de repli.

Cette tranche analyse le mono et la stéréo. L’analyse multicanale, les vumètres temps réel, LUFS-M/S, la corrélation et le spectre restent hors périmètre. La « lecture native » ne promet pas un flux bit-perfect après le mixeur du système. Aucune normalisation, égalisation, compression ni limitation n’est appliquée.

0.3.0-beta.1 a été compilée et testée sous Linux. Une validation Windows 10 native et la production des artefacts restent nécessaires avant publication.
