#!/bin/bash

umount /dev/nbd0
nbd-client -d /dev/nbd0
kill `cat /var/run/safedisk.PID`

