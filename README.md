safedisk
========

Building
--------
`./build`

Making a new disk 100 MB disk with backing storage in /tmp/hello and password HelloWorld
-----------------
`sudo ./mkdisk.sh /tmp/hello 100 HelloWorld`

Mounting the above disk in /tmp/mnt
----------------
`sudo ./mountdisk.sh /tmp/hello/ HelloWorld2 /tmp/mnt/`

Unmounting the above disk
------------------
`sudo ./unmountdisk.sh /tmp/hello

TODO
-------
Note, error handling on shell scripts in non-existant.  Also, Frank, I totally hacked the build code, probably not the way to meant to extend it.  Actual c/c++ code is pretty solid, one issue remaining regarding mkdir, which the shell script works around by making the directory before the c++ code gets to it


