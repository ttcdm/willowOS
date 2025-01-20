rm -rf iso_contents
mkdir iso_contents
bsdtar -x -f template.iso -C iso_contents

nm iso_contents/boot/kernel | grep kmain
readelf -S iso_contents/boot/kernel | grep debug
gdb iso_contents/boot/kernel
