# willowOS
A simple x86-64 operating system

Research period started during October 2024 and first lines of code was actually written during January 2025

All suggestions are welcome, as I am pretty inexperienced compared to the norm in this subject.

The kernel is inside main.c which is inside limine-c-template-x86-64

https://github.com/limine-bootloader/limine-c-template-x86-64  
https://github.com/mintsuki/flanterm

To run, git clone --recursive the repository and cd to limine-c-template-x86-64 and type in "make run".
./g-debug.sh to start up gdb debug after running make run-debug

Current progress:
GDT - DONE
IDT - Interrupts are set up and callable, but no APIC has been implemented yet.
Memory management - Memmap is set up and I'm currently writing some paging stuff.

# Bootloader development is paused for now as Limine is now used as the bootloader

./qb.sh to assemble bootloader and run it in QEMU
