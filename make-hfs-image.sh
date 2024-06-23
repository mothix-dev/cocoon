#!/bin/sh

# script to make compiling cocoon and making an HFS disk image for it easy

PLATFORM=openfirmware-powerpc CC=clang LD=ld.lld bmake

truncate -s 8M filesystem.img
./copy-hfs-files.sh filesystem.img
