#!/bin/bash

set -e

volume=$1
diskutil eject "$volume"
