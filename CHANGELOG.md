# Changelog

All notable changes to Audio A/B Comparator are documented in this file. Public versions use the straightforward `major.minor.patch` format.

## [Unreleased]

## [1.0.0] - 2026-08-22

### Changed

- published the first stable release, completing the intended Express, Blind Test and Analysis feature set;
- simplified public versioning to normal `major.minor.patch` releases without beta or release-candidate suffixes;
- documented mono and stereo as the supported analysis scope; multichannel tracks remain loadable but are not analysed;
- removed speculative roadmap items from the public presentation and updated all current download, packaging and security wording for the stable release.

## [0.3.0-beta.3] - 2026-08-22

### Added

- paired live Sample Peak, True Peak and RMS meters for A and B at the same playback position;
- per-track minimum and maximum markers retained from playback start until Stop or a selection change;
- EBU R 128 momentary (LUFS-M, 400 ms) and short-term (LUFS-S, 3 s) loudness, updated during playback and frozen on pause;
- serialized background live-analysis requests that never perform analysis or take a lock in the audio callback.

### Changed

- the Analysis dashboard now places the A/B mastering table and live meters side by side in two stable columns;
- all 150 interface strings are complete in each of the six translated catalogs, in addition to the English source.

## [0.3.0-beta.2] - 2026-08-22

### Changed

- Linux builds now prefer the desktop portal for the system file chooser while retaining Qt Quick Dialogs as a fallback when no session portal is available;
- the Analysis scope selector now uses an unambiguous active color and the shorter `All | Selection` labels;
- selection handles preview their range during dragging and trigger analysis only when released.

### Fixed

- the Analysis scroll view no longer steals selection-handle drags after a few pixels;
- the playback-time badge now sits in a dedicated lane above the playhead instead of covering the waveform.

## [0.3.0-beta.1] - 2026-08-21

### Added

- a third `Analysis` dashboard for whole-file and shared-selection mastering measurements: Sample Peak, True Peak, integrated loudness, Loudness Range, RMS, crest factor and DC offset;
- native PCM retention and background analysis for both audible and non-audible tracks, with debounced selection recalculation and stale-result rejection;
- statically linked libebur128 1.2.6 under the MIT license;
- source and playback format summaries for each track, including explicit sample-rate, channel and PCM sample-format conversions;
- synthetic coverage for silence, tones, DC offset, level changes, UInt8/Int16/Int32/Float, mono/stereo and playback-format decisions.

### Changed

- decoding now preserves each source PCM format first; identical supported formats are sent to Qt natively, otherwise both tracks are converted to the default device's preferred format;
- the top mode selector is now `Express | Blind Test | Analysis`; Analysis preserves transport, selection and votes and is blocked until the current Blind Test is exited.

### Documentation

- documented native-playback semantics, analysis limits, libebur128 integration and build requirements.

## [0.2.1-beta.4] - 2026-08-21

### Added

- built-in English and French interfaces with initial system-language detection;
- a persistent language selector in Settings, with immediate interface and audio-status retranslation;
- German, Spanish, Brazilian Portuguese, Japanese and Simplified Chinese interfaces, including automatic system-language detection.

## [0.2.1-beta.3] - 2026-08-21

### Added

- a dedicated A/B application icon matching the interface colour palette, embedded in Linux and Windows builds;
- Linux desktop metadata and a scalable application icon for desktop integration;
- a Linux x86_64 AppImage built on Ubuntu 22.04 LTS with Qt 6.9.3, FFmpeg, XCB and Wayland support;
- reproducible AppImage packaging scripts with AppStream metadata and automated WAV, FLAC and MP3 validation;
- public screenshots of the light theme and Blind Test screen.

### Documentation

- expanded the public GitHub presentation with the complete feature list, instant keyboard voting, planned features and an explicit invitation for feedback.

### Fixed

- KDE/Wayland task bars now resolve the branded A/B icon instead of the generic audio icon.
- the Linux AppImage now includes Qt's Wayland-EGL client integration, preventing an OpenGL/RHI startup crash under native Wayland.

## [0.2.1-beta.2] - 2026-08-21

### Added

- a clear popup explains that both tracks must be loaded before starting a Blind Test.

### Changed

- action buttons now use a modern rounded style with explicit hover, pressed, focus and highlighted states;
- the transient action text beside Settings has been removed from the header;
- track replacement controls are hidden throughout Blind Test mode;
- start/end selection now uses a clearly labelled control row between the two waveforms, while waveform dragging remains dedicated to the playhead;
- revealed Blind Test results now offer a single centered `Recommencer` action that resets the session, randomly selects a track and immediately resumes playback.

## [0.2.1-beta.1] - 2026-08-20

### Added

- first public source release under GPL-3.0-or-later;
- Windows 10 and Windows 11 x64 portable distribution;
- English-first public documentation, third-party notices and GitHub issue templates;
- reproducible Windows packaging script and SHA-256 manifest.

### Changed

- visible version now identifies the beta as `0.2.1-beta.1` while the technical project version is `0.2.1`;
- Windows validation documentation now covers Windows 10 and Windows 11 x64;
- binary archives are distributed only through GitHub Releases.

## 0.2.0 - 2026-08-20

### Added

- shared interactive timeline and waveform selection;
- optional transition beep with adjustable volume;
- Blind Test sessions with constrained random selection, separate scores and reveal;
- persistent light/dark theme and configurable shortcuts.

## 0.1.3 - 2026-08-19

### Added

- five-second keyboard navigation.

### Fixed

- playback start, stop and resume behaviour.

[Unreleased]: https://github.com/KarmaGame33/AudioABComparator/compare/v1.0.0...HEAD
[1.0.0]: https://github.com/KarmaGame33/AudioABComparator/releases/tag/v1.0.0
[0.3.0-beta.3]: https://github.com/KarmaGame33/AudioABComparator/releases/tag/v0.3.0-beta.3
[0.3.0-beta.2]: https://github.com/KarmaGame33/AudioABComparator/releases/tag/v0.3.0-beta.2
[0.3.0-beta.1]: https://github.com/KarmaGame33/AudioABComparator/releases/tag/v0.3.0-beta.1
[0.2.1-beta.4]: https://github.com/KarmaGame33/AudioABComparator/releases/tag/v0.2.1-beta.4
[0.2.1-beta.3]: https://github.com/KarmaGame33/AudioABComparator/releases/tag/v0.2.1-beta.3
[0.2.1-beta.2]: https://github.com/KarmaGame33/AudioABComparator/releases/tag/v0.2.1-beta.2
[0.2.1-beta.1]: https://github.com/KarmaGame33/AudioABComparator/releases/tag/v0.2.1-beta.1
