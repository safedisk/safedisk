safedisk
========

Building on OS X
--------
1) Install osxfuse recent version
2) `brew install openssl`
3) `. ./os
4) `./build`

Note: brew has osxfuse, should I use that?  What about packaging?  I need to make a pkg with all the deps.


Making a new disk 100 MB disk with backing storage in ~/Documents/safedisk_store
-----------------
`./scripts/macosx/make_safedisk ~/Documents/safedisk_store 100`

Safedisk will prompt you for a password, this will be your new disk's password

Mounting the above disk
----------------
`./scripts/macosx/mount_safedisk ~/Documents/safedisk_100`

Safedisk will again prompt you for a password.  Presuming it's correct, the system will mount the drive
and it will appear as a mounted volume (initally called Untitled).

Unmounting the above disk
------------------
Hit the eject icon for the disk

Copying data elsewhere (backup)
------------------
`~/Documents/safedisk_store/data` contains all of the actual data, rsync at will.

TODO
-------
Put TODO list into github
Improper error handling on bad password (just fails to mount, no good error)
After ejecting disk, you need to do 
`umount ~/Documents/safedisk_store/fuse_mnt`
Remove mkdir code from c++ code
Fix linux version (which was working)


