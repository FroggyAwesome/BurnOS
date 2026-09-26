#ifndef IO_H
#define IO_H

void outb(unsigned short port, unsigned char val);
void outw(unsigned short port, unsigned short val);
unsigned char inb(unsigned short port);
void io_wait(void);

#endif
