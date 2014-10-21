#!/bin/sh

sudo installer -dumplog -verbose -pkg "$1/Contents/Resources/osxfuse-2.7.2.pkg" -target /
