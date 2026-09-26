#include "pic.h"
#include "io.h"

void
pic_remap(int offset1, int offset2)
{
	unsigned char a1 = inb(0x21);
	unsigned char a2 = inb(0xA1);

	outb(0x20, 0x11);
	io_wait();
	outb(0xA0, 0x11);
	io_wait();
	outb(0x21, offset1);
	io_wait();
	outb(0xA1, offset2);
	io_wait();
	outb(0x21, 0x04);
	io_wait();
	outb(0xA1, 0x02);
	io_wait();
	outb(0x21, 0x01);
	io_wait();
	outb(0xA1, 0x01);
	io_wait();

	outb(0x21, a1);
	outb(0xA1, a2);
}

void
pic_send_end_of_interrupt(unsigned char irq)
{
	if (irq >= 8) {
		outb(0xA0, 0x20);
	}
	outb(0x20, 0x20);
}
