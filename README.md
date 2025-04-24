# willowOS
A simple x86-64 operating system<br>

Research started in October 2024 and the first lines of code were written during January 2025.<br>
All suggestions are welcome, as I am pretty inexperienced compared to the norm in this subject.<br>
The kernel source files are inside `limine-c-template-x86-64/kernel/src`<br>

## Current progress:
**GDT** - Works I'm pretty sure<br>
**IDT** - I can expand on the entries as I see fit, but it works fine.<br>
**Memory Management** - Memory mapping, physical and virtual addressing, physical page frame allocation and deallocation, and a functional heap. Missing virtual memory unmapping.<br>
**ACPI** - Parsed APCI tables manually. Will switch to using uACPI in the future. Local APIC is enabled and working. Timers are working ish as well. Also need an IOAPIC for keyboard interrupts so I don't have to poll for inputs.<br>
**Multiprocessing** - Currently using 4 physical cores. AP's are properly initialized and are executing C code. Their local APIC's are enabled as well. Need to implement spinlocks like sephamores or something, processor synchronization, something to do with caching idk.<br>
**Scheduling** - A working preemptive context switching scheduler. Currently implementing sleeping, blocking, and yielding, but the processes creating, pushing, and running threads are all functional.<br>

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
