# Résumé de version 0.3.0-beta.1

**Date :** 21 août 2026  
**Version technique :** 0.3.0  
**Version affichée et préparée :** 0.3.0-beta.1  
**Statut :** première préversion 0.3 de développement, non publiée

## Résultat

Cette préversion introduit un troisième écran **Analyse** consacré aux mesures de mastering sur le PCM natif. Elle sépare désormais le format source conservé pour l’analyse du format effectivement envoyé au périphérique audio. Lorsque A et B partagent un format PCM accepté, la lecture utilise directement ce format ; sinon, l’application détaille les conversions réalisées pour chaque piste.

## Contenu de la version

- analyses fichier entier et sélection pour A et B : Sample Peak, True Peak, LUFS-I, LRA, RMS, facteur de crête et décalage DC ;
- `AnalysisMetrics` fortement typé et contrôleur Qt Concurrent avec stabilisation de 250 ms et rejet des résultats obsolètes ;
- conservation des buffers PCM natifs et analyse de la piste audible comme de la piste non audible ;
- décision de lecture testable : format natif commun accepté, sinon format préféré du périphérique ;
- détail séparé des changements de fréquence, disposition des canaux et type d’échantillon ;
- libebur128 1.2.6 liée statiquement, archive officielle verrouillée par SHA-256 et licence MIT embarquée ;
- sélecteur `Express | Blind Test | Analyse`, transport condensé et blocage explicite pendant un Blind Test ;
- sélecteur de fichiers système préféré via `xdg-desktop-portal` dans tous les builds Linux, sans supprimer le repli Qt sur les systèmes qui n’exposent pas de portail ;
- 137 chaînes terminées dans chacun des six catalogues traduits, plus l’anglais source.

## Validation effectuée sous Linux

- build Debug et Release avec Qt 6.11.1, CMake/Ninja et libebur128 statique ;
- CTest : 30 scénarios réussis, dont UInt8, Int16, Int32 et Float en mono/stéréo, silence, sinus pleine échelle et atténué, offset DC, variation de niveau, décisions de lecture et invalidation asynchrone ;
- régression WAV, FLAC et MP3 avec lecture, sélection, bascule, votes et Blind Test ;
- smoke tests réussis dans les sept langues ;
- décision de thème Linux testée pour le portail disponible, le portail indisponible, l’absence de bus, l’absence du plugin et le respect d’un thème explicitement choisi par l’utilisateur ;
- sélecteur de fichiers KDE obtenu via `xdg-desktop-portal` et contrôlé visuellement sous Wayland, avec démarrage de secours validé sans portail ;
- audit de liaison : aucun objet partagé libebur128, symboles `ebur128_*` présents dans l’exécutable.

Les tolérances numériques sont documentées dans `docs/analysis_validation.md`.

## Limites et validation restante

- analyse limitée au mono et à la stéréo ; le multicanal est signalé comme non pris en charge ;
- la lecture native signifie « format PCM commun remis à Qt », sans promesse bit-perfect après PipeWire, Windows Audio ou le mixeur système ;
- aucune normalisation, égalisation, compression ni limitation n’est appliquée ;
- la reconstruction et la validation natives Windows 10 de 0.3.0-beta.1 restent à effectuer avant toute publication ;
- l’AppImage Linux 0.3.0-beta.1 a été reconstruite localement ; aucun tag ni aucune release n’a été créé ou publié.
