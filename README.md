# willowOS
A simple x86-64 operating system<br>

Research started in October 2024 and the first lines of code were written during January 2025.<br>
All suggestions are welcome, as I am pretty inexperienced compared to the norm in this subject.<br>
The kernel source files are inside `limine-c-template-x86-64/kernel/src`<br>

## Current progress:
GDT - DONE<br>
IDT - Interrupts are set up and callable. Refer to the ACPI entry for more info.<br>
Memory management - Memory mapping, physical and virtual addressing, physical page frame allocation and deallocation, and a functional heap.<br>
ACPI - Parsed APCI tables manually. Will switch to using uACPI in the future. Local APIC is enabled and working, but have yet to get it set up for other physical cpu cores.
Multiprocessing - Currently using 4 physical cores. Currently trying to get the application processors to execute C code. They can start up but I'm still trying to work out the assembly so I can set up their local APIC's and send IPI's and have a proper working system.

### To run,<br>
```
git clone --recursive https://github.com/ttcdm/willowOS.git
cd limine-c-template-x86-64
make run
```
<br>
<del>./g-debug.sh to start up gdb debug after running make run-debug</del><br>

### Credits:<br>
https://github.com/limine-bootloader/limine-c-template-x86-64<br>
https://github.com/mintsuki/flanterm<br>

### Bootloader development is paused for now as Limine is now used as the bootloader

<del>./qb.sh to assemble bootloader and run it in QEMU</del><br>
