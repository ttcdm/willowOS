#!/bin/bash
rm -rf iso_contents
mkdir iso_contents
bsdtar -x -f template.iso -C iso_contents
eu-addr2line -e ./iso_contents/boot/kernel $1
