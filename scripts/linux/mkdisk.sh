#!/bin/bash

usage ()
{
	echo 'Usage : mksafe <dir> <size> <key>'
	echo '   <dir> = directory to store disk files in'
	echo '   <size> = size in megabytes'
	echo '   <key> = crypto secret key'
	exit
}

if [ "$#" -ne 3 ]
then
	usage
fi

dir=$1
size=$2
key=$3

mkdir $1
echo $2 > $1/size
nbdkit -P /var/run/safedisk.PID ./out/release/safe_disk.so dir=$dir size=$size key=$key
nbd-client localhost 10809 /dev/nbd0
mkfs.ext3 /dev/nbd0
nbd-client -d /dev/nbd0
kill `cat /var/run/safedisk.PID`


