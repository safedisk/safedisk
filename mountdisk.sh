#!/bin/bash

usage ()
{
	echo 'Usage : mountdisk <dir> <key> <mp>'
	echo '   <dir> = directory to store disk files in'
	echo '   <key> = crypto secret key'
	echo '   <mp> = mount point, ie, directory to mount to'
	exit
}

if [ "$#" -ne 3 ]
then
	usage
fi

dir=$1
key=$2
mp=$3

size=`cat $1/size`
nbdkit -P /var/run/safedisk.PID ./out/release/safe_disk.so dir=$dir size=$size key=$key
nbd-client localhost 10809 /dev/nbd0
mount /dev/nbd0 $mp


