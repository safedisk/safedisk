#!/bin/bash +x

set -e

system_path=$1

mkdir -p "$system_path/fuse"
./safediskd "$system_path/fuse" "$system_path/disk"

on_exit1() {
    umount "$system_path/fuse"
}

on_exit2() {
    touch "$system_path/fuse/shutdown" || true
}

trap on_exit1 EXIT

volume=$(hdiutil attach -imagekey diskimage-class=CRawDiskImage "$system_path/fuse/data" | cut -f3-)

trap on_exit2 EXIT

rm -f "$system_path/volume"
ln -s "$volume" "$system_path/volume"
