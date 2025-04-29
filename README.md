# willowOS
A simple x86-64 operating system<br>

Research started in October 2024 and the first lines of code were written during January 2025.<br>
All suggestions are welcome, as I am pretty inexperienced compared to the norm in this subject.<br>
The kernel source files are inside `limine-c-template-x86-64/kernel/src`<br>

## Current progress:
**GDT** - Works I'm pretty sure<br>
**IDT** - I can expand on the entries as I see fit, but it works fine.<br>
**Memory Management** - Memory mapping, physical and virtual addressing, physical page frame allocation and deallocation, and a functional heap. Missing virtual memory unmapping.<br>
**ACPI** - Parsed APCI tables manually. Will switch to using uACPI in the future. Local APIC is enabled and working. Timers are working ish as well. Currently working on hooking up the IOAPIC to be able to receive keyboard interrupts instead of having to poll for them.<br>
**Multiprocessing** - Currently using 4 physical cores. AP's are properly initialized and are executing C code. Their local APIC's are enabled as well. Need to implement spinlocks like sephamores or something, processor synchronization, something to do with caching idk.<br>
**Scheduling** - A working preemptive context switching scheduler. Able to create, push, run, block, unblock, and yield threads. A priority system hasn't been implemented yet so every thread gets equal CPU time unless they voluntarily yield early. Currently trying to get thread sleeping to work.<br>
**Miscellaneous** - Spinlocks and mutexes haven't been implemented yet. It should be easy but I've been putting it off.<br>

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
https://github.com/charlesnicholson/nanoprintf<br>

### Bootloader development is paused for now as Limine is now used as the bootloader

<del>./qb.sh to assemble bootloader and run it in QEMU</del><br>
