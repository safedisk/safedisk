#!/bin/bash

set -e

storage_path=$1
size_MB=$(cat "$storage_path/size")

./safediskd "$storage_path/fuse" "$storage_path/blocks" $size_MB

on_exit() {
    touch "$storage_path/fuse/shutdown" || true
}
trap on_exit EXIT

hdiutil attach -imagekey diskimage-class=CRawDiskImage "$storage_path/fuse/data" | awk '{print $2}'
