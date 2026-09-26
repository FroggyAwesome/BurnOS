#include "keyboard.h"
#include "dev.h"
#include "io.h"
#include "vga.h"

static unsigned char buffer[BUFFER_SIZE];
static volatile int head = 0;
static volatile int tail = 0;
static int shifted = 0;

void keyboard_handler(void);

void
keyboard_handler(void)
{
	unsigned char code = inb(0x60);
	if (code == 0x2A || code == 0x36) {
		shifted = 1;
		return;
	}
	if (code == 0xAA || code == 0xB6) {
		shifted = 0;
		return;
	}

	int next = (head + 1) % BUFFER_SIZE;
	if (next != tail) {
		buffer[head] = code;
		head = next;
	}
}

unsigned char
get_scancode(void)
{
	while (head == tail) {
		__asm__ volatile("sti; hlt");
	}
	unsigned char code = buffer[tail];
	tail = (tail + 1) % BUFFER_SIZE;
	return code;
}

void
keyboard_flush(void)
{
	head = 0;
	tail = 0;
	while (inb(0x64) & 1) {
		inb(0x60);
		__asm__ volatile("pause");
	}
}

char
scancode_to_ascii(unsigned char code)
{
	static const char map[2][128] = {
		{ 0, 27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
			'\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
			0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',
			0, '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0,
			'*', 0, ' ', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			'7', '8', '9', '-', '4', '5', '6', '+', '1', '2', '3', '0', '.', 0 },
		{ 0, 27, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b',
			'\t', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n',
			0, 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~',
			0, '|', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', 0,
			'*', 0, ' ', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			'7', '8', '9', '-', '4', '5', '6', '+', '1', '2', '3', '0', '.', 0 }
	};
	if (code & 0x80)
		return 0;
	return map[shifted][code & 0x7F];
}

void
read_line(char *buf, int max_len)
{
	int i = 0;
	while (i < max_len - 1) {
		char c = scancode_to_ascii(get_scancode());
		if (!c)
			continue;
		if (c == '\n') {
			vga_putchar('\n');
			break;
		}
		if (c == '\b') {
			if (i > 0) {
				i--;
				buf[i] = '\0';
				vga_putchar('\b');
				vga_putchar(' ');
				vga_putchar('\b');
			}
			continue;
		}
		if (c >= 32 && c <= 126) {
			buf[i++] = c;
			vga_putchar(c);
		}
	}
	buf[i] = '\0';
}
