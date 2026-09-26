#include "idt.h"

struct idt_entry idt[256];
struct idt_ptr ip;

extern void irq0_handler_asm(void);
extern void irq1_handler_asm(void);

void
idt_set_gate(unsigned char num, unsigned long base, unsigned short sel, unsigned char flags)
{
	idt[num].offset_low = (base & 0xFFFF);
	idt[num].selector = sel;
	idt[num].zero = 0;
	idt[num].type_attr = flags;
	idt[num].offset_high = (base >> 16) & 0xFFFF;
}

void
idt_init(void)
{
	ip.limit = (sizeof(struct idt_entry) * 256) - 1;
	ip.base = (unsigned int)&idt;

	for (int i = 0; i < 256; i++) {
		idt_set_gate(i, 0, 0x10, 0x00);
	}

	idt_set_gate(32, (unsigned long)irq0_handler_asm, 0x10, 0x8E);
	idt_set_gate(33, (unsigned long)irq1_handler_asm, 0x10, 0x8E);

	__asm__ volatile("lidt %0" : : "m"(ip));
}
