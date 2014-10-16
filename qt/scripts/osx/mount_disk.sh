#!/bin/bash +x

set -e

storage_path=$1

mkdir -p "$storage_path/fuse"
./safediskd "$storage_path/fuse" "$storage_path/blocks"

on_exit1() {
    umount "$storage_path/fuse"
}

on_exit2() {
    touch "$storage_path/fuse/shutdown" || true
}

trap on_exit1 EXIT

volume=$(hdiutil attach -imagekey diskimage-class=CRawDiskImage "$storage_path/fuse/data" | cut -f3-)

trap on_exit2 EXIT

rm -f "$storage_path/volume"
ln -s "$volume" "$storage_path/volume"
