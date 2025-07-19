# willowOS
A simple x86-64 operating system<br>

Research started in October 2024 and the first lines of code were written during January 2025.<br>
All suggestions are welcome, as I am pretty inexperienced compared to the norm in this subject.<br>
The kernel source files are inside `limine-c-template-x86-64/kernel/src`<br>

System requirements: whatever `Limine`, `mlibc`, and `uACPI` requires.<br>

## Current progress:
**GDT** - Works I'm pretty sure.<br>
**IDT** - I can expand on the entries as I see fit, but it works fine.<br>
**Memory Management** - Memory mapping, physical and virtual addressing, physical page frame allocation and deallocation, and a functional heap.<br>
**ACPI** - Switched to uACPI for parsing ACPI tables, but I'm only using the uACPI barebones mode because I haven't implemented the full suite of uACPI kernel_api.h functions yet. <del>Parsed APCI tables manually. Will switch to using uACPI in the future.</del> Local APIC is enabled and working. Timers are working ish as well. Keyboard input also works via the IOAPIC.<br>
**Multiprocessing** - Currently using 4 physical cores. AP's are properly initialized and are executing C code. Their local APIC's are enabled as well. Currently trying to adapt the scheduler to be able to spread the load across all cores.<br>
**Scheduling** - A working preemptive context switching scheduler that uses a circular doubly linked list for a queue. Able to create, push, run, block, unblock, yield, and exit threads. A priority system hasn't been implemented yet so every thread gets equal CPU time unless they voluntarily yield early. Currently trying to get thread sleeping to work. I'm planning to add SMP support for it after I finish porting mlibc.<br>
**VFS** - Functional tmpfs with a working vfs layer with vnodes representing each file object. USTAR functionalities have also been ported.<br>
**Syscalls** - Using `syscall` and `sysret` for syscalls. The system for syscalls are fully functional, and I'm currently porting mlibc with the said syscalls. Most of the basic syscalls are just wrapped `vnode_ops`.<br>
**ELF** - ELF program loading works and they are able to be threaded and ran by the scheduler in userspace.<br>
**Miscellaneous** - Basic spinlocks and mutexes are implemented. Almost all functions also aren't wrapped with asserts even though assert is implemented.<br>
**Things that still need to be fixed** - Rework the bitmap for each memmap for pmm so it's a proper bitmap. Implement the rest of the VFS functions. Better mutex support? It currently only uses spinlocks and i don't exactly remember the distinctions between the two. Still need to fix timed thread sleeping, but thread blocking sorta suffices sometimes i think. Add return values to a lot of the kernel functions for error checking and stuff. Fix tmpfs.<br>

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
https://github.com/uACPI/uACPI<br>
https://github.com/managarm/mlibc<br>

### Bootloader development is paused for now as Limine is now used as the bootloader

<del>./qb.sh to assemble bootloader and run it in QEMU</del><br>
