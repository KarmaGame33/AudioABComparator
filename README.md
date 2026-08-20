# Audio A/B Comparator

**Free and open-source A/B audio comparator for mixing, mastering and blind listening tests.**

Audio A/B Comparator is a portable desktop application by **KarmaApps by KarmaGame**. Load two local audio files, select one shared passage and switch instantly between A and B without uploading your music. The application contains no telemetry and does not retain audio paths, votes or listening sessions.

![Audio A/B Comparator interface](docs/media/audio-ab-comparator.png)

![A/B switching and Blind Test demonstration](docs/media/audio-ab-comparator-demo.gif)

## Download

Version **0.2.1-beta.1** supports **Windows 10 and Windows 11 x64**. Download the portable ZIP and `SHA256SUMS` from the [GitHub Releases page](https://github.com/KarmaGame33/AudioABComparator/releases). Do not download binaries from the repository source tree.

This beta is unsigned. Windows SmartScreen may display a warning because the executable has no Authenticode signature or established reputation. Verify the SHA-256 checksum, extract the complete ZIP, then run `ab-compare.exe` from the extracted folder.

## Features

- local WAV, FLAC, MP3, AIFF and Ogg loading through Qt Multimedia;
- shared interactive timeline, waveform overview and selection;
- play, pause, stop, loop and five-second navigation;
- instant A/B switching with a short anti-click crossfade;
- optional A/B transition beep with adjustable volume;
- constrained random Blind Test sessions and score reveal;
- configurable shortcuts and persistent light/dark theme;
- no account, cloud upload or telemetry.

## Build on Linux

Qt 6.9 or later, CMake 3.24 or later, Ninja and a C++20 compiler are required. On Arch Linux:

```fish
sudo pacman -S --needed base-devel cmake ninja qt6-base qt6-declarative qt6-multimedia qt6-multimedia-ffmpeg qt6-wayland
cmake -S . -B build/linux-release -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build/linux-release --parallel (nproc)
ctest --test-dir build/linux-release --output-on-failure
```

See [`docs/installs.md`](docs/installs.md) for Windows and Linux build details. A Linux AppImage is planned after validation on Ubuntu LTS, KDE/Wayland/PipeWire and Ubuntu GNOME.

## Project and contributions

This GitHub repository is the official public **mirror**. Development remains canonical in the private SVN master, and each public Git tag corresponds to an immutable SVN release tag. Issues are the feedback channel for bugs and feature requests. Pull requests are not processed for now; see [`CONTRIBUTING.md`](CONTRIBUTING.md).

Audio A/B Comparator is licensed under **GPL-3.0-or-later**. Runtime dependency notices and source links are in [`THIRD_PARTY_NOTICES.md`](THIRD_PARTY_NOTICES.md).

---

## Résumé français

**Audio A/B Comparator — KarmaApps par KarmaGame** est un comparateur audio A/B libre destiné au mixage, au mastering et aux tests d’écoute à l’aveugle. Les fichiers restent sur votre ordinateur : aucun compte, aucun envoi dans le cloud et aucune télémétrie.

La bêta **0.2.1-beta.1** est proposée en ZIP portable pour **Windows 10 et Windows 11 x64** sur la page [GitHub Releases](https://github.com/KarmaGame33/AudioABComparator/releases). Elle n’est pas signée ; vérifiez `SHA256SUMS`, extrayez tout le dossier, puis lancez `ab-compare.exe`. Les retours se font uniquement par les [Issues GitHub](https://github.com/KarmaGame33/AudioABComparator/issues).
