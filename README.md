SafeDisk
========

# What is SafeDisk?

SafeDisk is a cryptographically secure backup system, where you store copies of your important files (encrypted so no one can read them), on your other machines, or your friends machines.  The cryptography in safedisk uses standard techniques which have been proven safe, along with a special method to allow incrementalbackups without revealing which files changed, the details of which can be found in the [Crypto]() section.  A SafeDisk drive looks like a normal disk (similar to a USB drive), and is easy to set up via a menu widgit.  Currently, we only support Mac OS X, and Linux, but a Windows version is possible if enough people are interested.

# How to get SafeDisk

If you're a paranoid crypto type, a developer, or you run linux, we recomend you build from source (see [Building][] section).  For normal OS X users, we provide a package, which you can download here: [TODO: Insert link]

# How to use SafeDisk

[TODO: Newbie intro]

# How to set up a NAS to backup your data using BTSync

# How to backup to your friends using BTSync

# Crypto

SafeDisk is designed to prevent both simple attacks, such as an attacker reading your data, as well as more sophisticated ones, such as an attacker modifying your data with you knowing, or even a friend you back up your data to trying to spy of which data your editing.   
 
In addition, SafeDisk must support *incremental* backup.  That is, when you change only a small amount of data, the cost of updating your backups should be small.  To do this, SafeDisk combines two ideas:  GCM mode block encryption, and append only databases.

To explain the details of GCM encryption would take us too far afield, but please see the [wikipedia](http://en.wikipedia.org/wiki/Galois/Counter_Mode).  Of course GCM mode needs a block cipher, and SafeDisk uses [AES 256](http://en.wikipedia.org/wiki/Advanced_Encryption_Standard).  The precise details of how these well studied cryptographic primitives are used covered in [Gory Details][] for those who are intested in such things.

Regarding the notion of append only databases [TODO: Write this]

## Gory Details
[TODO: Write this] 

# Building

## Building on Linux

Some instructions here

### Linux build Status
[![Build Status](https://travis-ci.org/safedisk/safedisk.svg)](https://travis-ci.org/safedisk/safedisk)

## Building on OS X
1. `brew install openssl osxfuse qt5`
1. `. ./osx.env`
1. `./build package`

The result is at `out/release/SafeDisk.dmg`

Note: On OS X Yosemite, the `osxfuse` package is no longer installable via homebrew. You'll need to manually install this package on your development machine in order to build SafeDisk.

## How to use SafeDisk from the command line on OS X

### Make a 100 MB SafeDisk
`./scripts/macosx/make_safedisk ~/Documents/SafeDisk.disk 100`

SafeDisk will prompt you for a password, this will be your new disk's password.

### Mount a SafeDisk
`./scripts/macosx/mount_safedisk ~/Documents/SafeDisk.disk`

SafeDisk will again prompt you for a password.  
Presuming it's correct, the system will mount the drive
and it will appear as a mounted volume (initially called Untitled).

### Unmount a SafeDisk
Hit the eject icon for the disk.  

### Make a backup
`~/Documents/SafeDisk.disk/data` contains all of the actual data, rsync at will.


