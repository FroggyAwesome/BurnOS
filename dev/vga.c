#include "vga.h"
#include "io.h"

static unsigned short *const vga_buffer = (unsigned short *)VGA_BUFFER;
static int cursor_x = 0;
static int cursor_y = 0;
static unsigned char current_color = DEFAULT_COLOR;

static inline void
update_cursor(void)
{
	unsigned short pos = (unsigned short)(cursor_y * VGA_WIDTH + cursor_x);
	outb(0x3D4, 14);
	outb(0x3D5, (unsigned char)(pos >> 8));
	outb(0x3D4, 15);
	outb(0x3D5, (unsigned char)pos);
}

void
vga_hide_cursor(void)
{
	outb(0x3D4, 0x0A);
	unsigned char current = inb(0x3D5);
	outb(0x3D4, 0x0A);
	outb(0x3D5, current | 0x20);
}

void
vga_show_cursor(void)
{
	outb(0x3D4, 0x0A);
	unsigned char current = inb(0x3D5);
	outb(0x3D4, 0x0A);
	outb(0x3D5, (unsigned char)(current & ~0x20U));
}

void
vga_set_color(unsigned char bg, unsigned char fg)
{
	current_color = ((bg & 0x07) << 4) | (fg & 0x0F);
}

void
vga_reset_color(void)
{
	current_color = DEFAULT_COLOR;
}

static void
scroll(void)
{
	__asm__ volatile("cld\n\t"
									 "rep movsw"
									 :
									 : "S"(vga_buffer + VGA_WIDTH), "D"(vga_buffer), "c"(VGA_WIDTH * (VGA_HEIGHT - 1))
									 : "memory", "cc");

	unsigned short val = (current_color << 8) | ' ';
	__asm__ volatile("rep stosw" : : "D"(vga_buffer + VGA_WIDTH * (VGA_HEIGHT - 1)), "a"(val), "c"(VGA_WIDTH) : "memory");

	cursor_y = VGA_HEIGHT - 1;
}

void
vga_clear_screen(void)
{
	unsigned short val = (current_color << 8) | ' ';
	__asm__ volatile("rep stosw" : : "D"(vga_buffer), "a"(val), "c"(VGA_WIDTH * VGA_HEIGHT) : "memory");
	cursor_x = 0;
	cursor_y = 0;
	update_cursor();
}

void
vga_putchar_color(char c, unsigned char color)
{
	if (c == '\n') {
		cursor_x = 0;
		cursor_y++;
	} else if (c == '\b') {
		if (cursor_x > 0) {
			cursor_x--;
			vga_buffer[cursor_y * VGA_WIDTH + cursor_x] = (color << 8) | ' ';
		}
	} else {
		vga_buffer[cursor_y * VGA_WIDTH + cursor_x] = (color << 8) | c;
		cursor_x++;
	}
	if (cursor_x >= VGA_WIDTH) {
		cursor_x = 0;
		cursor_y++;
	}
	if (cursor_y >= VGA_HEIGHT) {
		scroll();
	}
	update_cursor();
}

void
vga_putchar(char c)
{
	vga_putchar_color(c, current_color);
}

void
vga_print_color(const char *str, unsigned char color)
{
	while (*str) {
		vga_putchar_color(*str++, color);
	}
}

void
vga_print(const char *str)
{
	vga_print_color(str, current_color);
}

unsigned short *
vga_get_buffer(void)
{
	return vga_buffer;
}

int
vga_get_cursor_x(void)
{
	return cursor_x;
}

int
vga_get_cursor_y(void)
{
	return cursor_y;
}

void
vga_set_cursor(int x, int y)
{
	cursor_x = x;
	cursor_y = y;
	update_cursor();
}

void
vga_set_cursor_form(char type)
{
	unsigned char start = 13;
	unsigned char end = 14;

	if (type == 'b') {
		start = 0;
		end = 15;
	} else if (type == 'h') {
		start = 8;
		end = 15;
	} else if (type == 'l') {
		start = 13;
		end = 14;
	}

	outb(0x3D4, 0x0A);
	outb(0x3D5, start);
	outb(0x3D4, 0x0B);
	outb(0x3D5, end);
}

void
vga_putchar_at(int x, int y, char c, unsigned char color)
{
	if (x < 0 || x >= VGA_WIDTH || y < 0 || y >= VGA_HEIGHT)
		return;

	vga_buffer[y * VGA_WIDTH + x] = ((unsigned short)color << 8) | (unsigned char)c;
}
