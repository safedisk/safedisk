#!/bin/bash

set -e

storage_path=$1
name=$2
size_MB=$3

mkdir -p "$storage_path/fuse"
mkdir -p "$storage_path/blocks"
echo $size_MB > "$storage_path/size"

./safediskd "$storage_path/fuse" "$storage_path/blocks" $size_MB

on_exit() {
    hdiutil detach $device || true
    umount "$storage_path/fuse"
}
trap on_exit EXIT

device=$(hdiutil attach -imagekey diskimage-class=CRawDiskImage -nomount "$storage_path/fuse/data" | awk '{print $1}')
newfs_hfs -v "$name" $device
