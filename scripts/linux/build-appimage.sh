#!/usr/bin/env bash

set -euo pipefail

source_dir=${1:-/src}
work_dir=${2:-/work}
output_dir=${3:-/output}

source_dir=$(realpath "$source_dir")
work_dir=$(realpath "$work_dir")
output_dir=$(realpath "$output_dir")

test -f "$source_dir/CMakeLists.txt"
test -f "$source_dir/LICENSE"
test -f "$source_dir/THIRD_PARTY_NOTICES.md"
test -f "$source_dir/packaging/linux/io.github.KarmaGame33.AudioABComparator.metainfo.xml"
test -d "$work_dir"
test -d "$output_dir"

release_version=$(sed -n 's/^set(AB_COMPARE_RELEASE_VERSION "\([^"]*\)")$/\1/p' "$source_dir/CMakeLists.txt")
if [[ ! $release_version =~ ^[0-9]+\.[0-9]+\.[0-9]+-beta\.[0-9]+$ ]]; then
    echo "Invalid release version: $release_version" >&2
    exit 2
fi

build_dir="$work_dir/build"
app_dir="$work_dir/AppDir"
home_dir="$work_dir/home"
appimage_name="AudioABComparator-${release_version}-linux-x86_64.AppImage"

for path in "$build_dir" "$app_dir"; do
    if [[ -e $path ]]; then
        echo "Build target already exists: $path" >&2
        exit 3
    fi
done

mkdir -p "$home_dir" "$app_dir" "$output_dir"
export HOME="$home_dir"

fixture_dir="$work_dir/format-fixtures"
mkdir -p "$fixture_dir"
for track_spec in "a:440" "b:660"; do
    track=${track_spec%%:*}
    frequency=${track_spec##*:}
    for extension in wav flac mp3; do
        ffmpeg -hide_banner -loglevel error -y \
            -f lavfi -i "sine=frequency=${frequency}:duration=8" \
            -ar 48000 -ac 2 "$fixture_dir/track-${track}.${extension}"
    done
done

cmake -S "$source_dir" -B "$build_dir" -G Ninja \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX=/usr \
    -DCMAKE_PREFIX_PATH="$QT_ROOT"
cmake --build "$build_dir" --parallel "$(nproc)"
AB_COMPARE_TEST_FORMAT_DIR="$fixture_dir" \
    ctest --test-dir "$build_dir" --output-on-failure
DESTDIR="$app_dir" cmake --install "$build_dir"

install -Dm644 \
    "$source_dir/packaging/linux/io.github.KarmaGame33.AudioABComparator.metainfo.xml" \
    "$app_dir/usr/share/metainfo/io.github.KarmaGame33.AudioABComparator.appdata.xml"
install -Dm644 "$source_dir/LICENSE" \
    "$app_dir/usr/share/doc/audio-ab-comparator/LICENSE"
install -Dm644 "$source_dir/THIRD_PARTY_NOTICES.md" \
    "$app_dir/usr/share/doc/audio-ab-comparator/THIRD_PARTY_NOTICES.md"
install -Dm644 "$source_dir/licenses/LGPL-2.1.txt" \
    "$app_dir/usr/share/doc/audio-ab-comparator/licenses/LGPL-2.1.txt"
install -Dm644 "$source_dir/licenses/LGPL-3.0.txt" \
    "$app_dir/usr/share/doc/audio-ab-comparator/licenses/LGPL-3.0.txt"

desktop-file-validate \
    "$app_dir/usr/share/applications/io.github.KarmaGame33.AudioABComparator.desktop"
appstreamcli validate --no-net \
    "$app_dir/usr/share/metainfo/io.github.KarmaGame33.AudioABComparator.appdata.xml"

export QMAKE="$QT_ROOT/bin/qmake"
export LD_LIBRARY_PATH="$QT_ROOT/lib${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"
export QML_SOURCES_PATHS="$source_dir/app/ui"
# linuxdeploy-plugin-qt associates the Wayland client graphics integrations
# (including wayland-egl) with its waylandcompositor module deployer.
export EXTRA_QT_MODULES="multimedia;svg;waylandcompositor"
export EXTRA_PLATFORM_PLUGINS="libqwayland-egl.so;libqwayland-generic.so"
export LINUXDEPLOY_OUTPUT_VERSION="$release_version"
export LDAI_OUTPUT="$output_dir/$appimage_name"

cd "$work_dir"
/usr/local/bin/linuxdeploy-x86_64.AppImage \
    --appdir "$app_dir" \
    --executable "$app_dir/usr/bin/ab-compare" \
    --desktop-file "$app_dir/usr/share/applications/io.github.KarmaGame33.AudioABComparator.desktop" \
    --icon-file "$app_dir/usr/share/icons/hicolor/scalable/apps/io.github.KarmaGame33.AudioABComparator.svg" \
    --plugin qt \
    --output appimage

test -x "$output_dir/$appimage_name"

# A QML smoke test exits before the first rendered frame. Inspect the package as
# well so a Wayland build cannot be published without its EGL client plugin.
inspection_dir="$work_dir/AppImageInspection"
mkdir "$inspection_dir"
(
    cd "$inspection_dir"
    "$output_dir/$appimage_name" --appimage-extract >/dev/null
)
test -f \
    "$inspection_dir/squashfs-root/usr/plugins/wayland-graphics-integration-client/libqt-plugin-wayland-egl.so"

(cd "$output_dir" && sha256sum "$appimage_name" > SHA256SUMS-linux)

echo "AppImage: $output_dir/$appimage_name"
cat "$output_dir/SHA256SUMS-linux"
