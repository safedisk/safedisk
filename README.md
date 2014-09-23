SafeDisk
========

Building on OS X
--------
1. `brew install openssl osxfuse`
1. `. ./osx.env`
1. `./build`

Make a 100 MB disk with backing storage in ~/Documents/SafeDisk.disk
-----------------
`./scripts/macosx/make_safedisk ~/Documents/SafeDisk.disk 100`

SafeDisk will prompt you for a password, this will be your new disk's password.

Mount a SafeDisk
------------
`./scripts/macosx/mount_safedisk ~/Documents/SafeDisk.disk`

SafeDisk will again prompt you for a password.  
Presuming it's correct, the system will mount the drive
and it will appear as a mounted volume (initally called Untitled).

Unmount a SafeDisk
------------------
Hit the eject icon for the disk

Make a backup
------------------
`~/Documents/SafeDisk.disk/data` contains all of the actual data, rsync at will.

TODO
-------
- [ ] Convert this list into GitHub issues
- [ ] Improper error handling on bad password (just fails to mount, no good error)
- [ ] After ejecting disk, you need to do 
- [ ] `umount ~/Documents/safedisk_store/fuse_mnt`
- [ ] Remove mkdir code from c++ code
- [ ] Fix linux version (which was working)
