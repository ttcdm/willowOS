# willowOS
A simple x86-64 operating system<br>

Research period started during October 2024 and first lines of code was actually written during January 2025<br>

All suggestions are welcome, as I am pretty inexperienced compared to the norm in this subject.<br>

The kernel is inside main.c which is inside limine-c-template-x86-64<br>

https://github.com/limine-bootloader/limine-c-template-x86-64<br>
https://github.com/mintsuki/flanterm<br>

To run, git clone --recursive the repository and cd to limine-c-template-x86-64 and type in "make run".<br>
./g-debug.sh to start up gdb debug after running make run-debug<br>

## Current progress:
GDT - DONE<br>
IDT - Interrupts are set up and callable, but no APIC has been implemented yet.<br>
Memory management - Memory mapping, physical page frame allocator, physical and virtual addressing, and a semi-functional heap.<br>

# Bootloader development is paused for now as Limine is now used as the bootloader

./qb.sh to assemble bootloader and run it in QEMU<br>
