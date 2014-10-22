SafeDisk
========

### Status
[![Build Status](https://travis-ci.org/safedisk/safedisk.svg)](https://travis-ci.org/safedisk/safedisk)

Building on OS X
--------
1. `brew install openssl osxfuse qt5`
1. `. ./osx.env`
1. `./build`

Creating a distributable .dmg
-------
1. Install [Iceberg](http://s.sudre.free.fr/Software/Iceberg.html)
1. `. ./osx.env`
1. `./build package`

The result is at `pkg/osx/build/SafeDisk.dmg`


Make a 100 MB SafeDisk
-----------------
`./scripts/macosx/make_safedisk ~/Documents/SafeDisk.disk 100`

SafeDisk will prompt you for a password, this will be your new disk's password.

Mount a SafeDisk
------------
`./scripts/macosx/mount_safedisk ~/Documents/SafeDisk.disk`

SafeDisk will again prompt you for a password.  
Presuming it's correct, the system will mount the drive
and it will appear as a mounted volume (initially called Untitled).

Unmount a SafeDisk
------------------
Hit the eject icon for the disk.  

Make a backup
------------------
`~/Documents/SafeDisk.disk/data` contains all of the actual data, rsync at will.
