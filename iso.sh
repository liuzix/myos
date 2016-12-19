#!/bin/bash
mkdir -p isofiles/boot/grub
cp bootloader isofiles/boot/kernel.bin
cp grub.cfg isofiles/boot/grub
grub-mkrescue -o myos.iso isofiles 2> /dev/null
rm -r isofiles 
