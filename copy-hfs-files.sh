#!/bin/sh

# script to format a disk image or disk as HFS and copy cocoon to it
# this disk image can then be booted with the command "boot hd:\\:tbxi" (substitute hd for the device being booted from)

umount $1
hformat $1
hmount $1

hmkdir cocoon
hattrib -b cocoon

hcopy cocoon :cocoon:cocoon
hattrib -t tbxi :cocoon:cocoon

hcopy initrd.tar :cocoon:initrd.tar

humount
sync
