#!/bin/bash

set -e

storage_path=$1
size_MB=$(cat "$storage_path/size")
name=$(basename "$storage_path" .disk)

./safediskd "$storage_path/fuse" "$storage_path/blocks" $size_MB

on_exit1() {
    umount "$storage_path/fuse"
}

on_exit2() {
    touch "$storage_path/fuse/shutdown" || true
}

trap on_exit1 EXIT

hdiutil attach -imagekey diskimage-class=CRawDiskImage "$storage_path/fuse/data"

trap on_exit2 EXIT

rm -f "$storage_path/volume"
ln -s "/Volumes/$name" "$storage_path/volume"
