#!/usr/bin/env bash

set -euo pipefail

script_dir=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)
repository_root=$(realpath "$script_dir/../..")
output_dir="$repository_root/dist/release"
builder_image="audioab-appimage-builder:ubuntu22.04-qt6.9.3"
build_root=$(mktemp -d "${TMPDIR:-/tmp}/audioab-appimage.XXXXXX")

cleanup()
{
    if [[ -n ${build_root:-} && -d $build_root ]]; then
        resolved_build_root=$(realpath "$build_root")
        case "$resolved_build_root" in
            /tmp/audioab-appimage.*)
                while IFS= read -r -d '' link_path; do
                    rm -- "$link_path"
                done < <(find "$resolved_build_root" -xdev -type l -print0)
                if [[ -n $(find "$resolved_build_root" -xdev -type l -print -quit) ]]; then
                    echo "Temporary build still contains a symbolic link; cleanup skipped: $resolved_build_root" >&2
                    return
                fi
                rm -rf -- "$resolved_build_root"
                ;;
            *)
                echo "Unexpected temporary build path; cleanup skipped: $resolved_build_root" >&2
                ;;
        esac
    fi
}
trap cleanup EXIT

mkdir -p "$build_root/work" "$output_dir"

docker build \
    --file "$repository_root/packaging/linux/Dockerfile.appimage" \
    --tag "$builder_image" \
    "$repository_root/packaging/linux"

docker run --rm \
    --volume "$repository_root:/src:ro" \
    --volume "$build_root/work:/work" \
    --volume "$output_dir:/output" \
    "$builder_image" \
    /src/scripts/linux/build-appimage-entrypoint.sh \
        "$(id -u)" "$(id -g)" /src /work /output
