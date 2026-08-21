# Third-party notices

This inventory covers the libraries distributed in the official Windows 10/11 x64 portable build of Audio A/B Comparator 0.2.1-beta.2. The application source itself is licensed under GPL-3.0-or-later.

## Qt 6.9.3

Runtime modules include Qt Core, GUI, Network, QML, Quick, Quick Controls, Multimedia and SVG, plus the platform and QML plugins selected by `windeployqt`.

- Copyright: The Qt Company Ltd. and other Qt contributors.
- License used for this distribution: GNU Lesser General Public License v3.0 only (`LGPL-3.0-only`). Qt also offers other licensing choices; those alternatives do not change this distribution's stated choice.
- License text: [`licenses/LGPL-3.0.txt`](licenses/LGPL-3.0.txt).
- Project: <https://www.qt.io/>
- Source for 6.9.3: <https://download.qt.io/official_releases/qt/6.9/6.9.3/submodules/>

Qt libraries remain separate dynamically linked DLLs in the portable package. Users may replace them with compatible modified builds, subject to the LGPL terms.

## FFmpeg 7.1.1 as supplied by Qt 6.9.3

The Qt Multimedia FFmpeg backend deploys `avcodec-61.dll`, `avformat-61.dll`, `avutil-59.dll`, `swresample-5.dll` and `swscale-8.dll`. Qt's 6.9.3 SBOM identifies this prebuilt dependency as FFmpeg tag `n7.1.1` and states that the online-installer binaries omit the optional GPL and LGPLv3 components.

- Copyright: 2000-2023 the FFmpeg developers and the respective component authors.
- Principal license: GNU Lesser General Public License v2.1 or later.
- Additional component licenses reported by the Qt SBOM: BSD-3-Clause, BSD-2-Clause, BSD Source Code Attribution, ISC, MIT and MPL-2.0. FFmpeg also incorporates limited code derived from libjpeg, zlib and Boost under their respective permissive licenses.
- LGPL text: [`licenses/LGPL-2.1.txt`](licenses/LGPL-2.1.txt).
- Project and legal information: <https://ffmpeg.org/> and <https://ffmpeg.org/legal.html>
- Exact source archive: <https://github.com/FFmpeg/FFmpeg/archive/refs/tags/n7.1.1.tar.gz>

The distributed FFmpeg DLLs are dynamically linked and may be replaced with compatible builds. Do not substitute a GPL-enabled FFmpeg build without reassessing the resulting licensing obligations.

## Signalsmith Stretch

Qt Multimedia contains Signalsmith Stretch for pitch compensation in its FFmpeg media backend.

- Copyright: Signalsmith Audio Ltd.
- License: MIT.
- Source: <https://github.com/Signalsmith-Audio/signalsmith-stretch>

## Mesa software OpenGL fallback

Qt's Windows deployment may include `opengl32sw.dll`, the Mesa software OpenGL fallback.

- License: MIT and other permissive licenses applying to individual Mesa components.
- Source and notices: <https://gitlab.freedesktop.org/mesa/mesa>

## Microsoft redistributable runtime files

The portable package may include Microsoft Visual C++ runtime DLLs and `D3Dcompiler_47.dll`, as selected by Qt's Windows deployment tool. They are redistributed under the applicable Microsoft Visual Studio and Windows SDK redistributable terms. They are not covered by the application's GPL license.

- Visual C++ redistribution: <https://learn.microsoft.com/en-us/cpp/windows/latest-supported-vc-redist>
- Windows SDK: <https://developer.microsoft.com/en-us/windows/downloads/windows-sdk/>

## Build-time tools

CMake, Ninja, MSVC and Qt's build tools are used to compile the application but are not incorporated as application source. `Qt6::Test` is used by the automated test executable and is not shipped in the end-user ZIP.

## Relinking and corresponding source

The public release tag contains the complete Audio A/B Comparator source and build scripts. The Qt and FFmpeg source links above identify the exact upstream versions used for the Windows package. Because the runtime libraries are separate DLLs, a recipient can replace them with compatible modified versions as permitted by their licenses.
