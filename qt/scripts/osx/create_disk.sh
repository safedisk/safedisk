#!/bin/bash -x

set -e

system_path=$1
storage_path=$2
size_MB=$3
guid=$(basename "$system_path")
name=$(basename "$storage_path" .disk)

mkdir -p "$system_path/fuse"

./safediskd "$system_path/fuse" "$storage_path" $size_MB

on_exit1() {
    hdiutil detach $device || true
    umount "$system_path/fuse"
}

on_exit2() {
    touch "$system_path/fuse/shutdown" || true
}

trap on_exit1 EXIT

device=$(hdiutil attach -imagekey diskimage-class=CRawDiskImage -nomount "$system_path/fuse/data" | awk '{print $1}')
newfs_hfs -v "$name" $device

hdiutil detach $device
volume=$(hdiutil attach -imagekey diskimage-class=CRawDiskImage "$system_path/fuse/data" | cut -f3-)

trap on_exit2 EXIT

rm -f "$system_path/disk"
ln -s "$storage_path" "$system_path/disk"
rm -f "$system_path/volume"
ln -s "$volume" "$system_path/volume"
echo $guid > "$storage_path/guid"
