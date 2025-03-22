#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include <kutils.h>


#define PIC1		0x20		/* IO base address for master PIC */
#define PIC2		0xA0		/* IO base address for slave PIC */
#define PIC1_COMMAND	PIC1
#define PIC1_DATA	(PIC1+1)
#define PIC2_COMMAND	PIC2
#define PIC2_DATA	(PIC2+1)

// https://wiki.osdev.org/8259_PIC#Disabling


// https://wiki.osdev.org/Inline_Assembly/Examples#I/O_access
static inline void outb(uint16_t port, uint8_t val);
static inline uint8_t inb(uint16_t port);

void pic_disable(void);