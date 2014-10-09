#!/bin/bash

set -e

storage_path=$1
size_MB=$2
name=$(basename "$storage_path" .disk)

mkdir -p "$storage_path/fuse"
mkdir -p "$storage_path/blocks"
echo $size_MB > "$storage_path/size"

./safediskd "$storage_path/fuse" "$storage_path/blocks" $size_MB

on_exit1() {
    hdiutil detach $device || true
    umount "$storage_path/fuse"
}

on_exit2() {
    touch "$storage_path/fuse/shutdown" || true
}

trap on_exit1 EXIT

device=$(hdiutil attach -imagekey diskimage-class=CRawDiskImage -nomount "$storage_path/fuse/data" | awk '{print $1}')
newfs_hfs -v "$name" $device

hdiutil detach $device
volume=$(hdiutil attach -imagekey diskimage-class=CRawDiskImage "$storage_path/fuse/data" | cut -f3-)

trap on_exit2 EXIT

rm -f "$storage_path/volume"
ln -s "$volume" "$storage_path/volume"
