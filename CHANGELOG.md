# Changelog

All notable changes to Audio A/B Comparator are documented in this file. The project follows Semantic Versioning; prerelease labels identify public beta builds.

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

[0.2.1-beta.3]: https://github.com/KarmaGame33/AudioABComparator/releases/tag/v0.2.1-beta.3
[0.2.1-beta.2]: https://github.com/KarmaGame33/AudioABComparator/releases/tag/v0.2.1-beta.2
[0.2.1-beta.1]: https://github.com/KarmaGame33/AudioABComparator/releases/tag/v0.2.1-beta.1
