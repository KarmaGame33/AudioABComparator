#!/usr/bin/env bash

set -euo pipefail

output_uid=${1:?Missing output UID}
output_gid=${2:?Missing output GID}
source_dir=${3:-/src}
work_dir=${4:-/work}
output_dir=${5:-/output}

if [[ ! $output_uid =~ ^[1-9][0-9]*$ || ! $output_gid =~ ^[1-9][0-9]*$ ]]; then
    echo "The build UID and GID must be positive integers." >&2
    exit 2
fi

pulse_dir=/tmp/audioab-pulse
install -d -m 0755 -o pulse -g pulse "$pulse_dir"
pulseaudio --system --daemonize=yes --disallow-exit=yes \
    --disallow-module-loading=yes --exit-idle-time=-1 \
    --load="module-native-protocol-unix auth-anonymous=1 socket=$pulse_dir/native" \
    --load=module-null-sink

test -S "$pulse_dir/native"
export PULSE_SERVER="unix:$pulse_dir/native"

exec setpriv \
    --reuid "$output_uid" \
    --regid "$output_gid" \
    --clear-groups \
    "$source_dir/scripts/linux/build-appimage.sh" \
        "$source_dir" "$work_dir" "$output_dir"
