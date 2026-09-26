#include "system.h"
#include "dev.h"
#include "io.h"
#include "keyboard.h"
#include "os.h"
#include "vga.h"

void
poweroff(void)
{
	outw(0x604, 0x2000);
	outw(0xB004, 0x2000);
	while (1)
		__asm__("hlt");
}

void
reboot(void)
{
	while (inb(0x64) & 2)
		;
	outb(0x64, 0xFE);
	while (1)
		__asm__("hlt");
}

void
halt(void)
{
	keyboard_flush();
	hide_cursor();
	print("System halted. Kernel stopped.\n");
	__asm__ volatile("cli");
	while (1) {
		__asm__ volatile("hlt");
	}
}

void
panic(const char *message)
{
	__asm__ volatile("cli");
	hide_cursor();

	unsigned short *vga_buffer = get_vga_buffer();
	unsigned char panic_bg = VGA_ENTRY_COLOR(VGA_BG_RED, VGA_FG_WHITE);
	unsigned char panic_footer = VGA_ENTRY_COLOR(VGA_BG_RED, VGA_FG_BLACK);

	for (int y = 0; y < VGA_HEIGHT; y++) {
		for (int x = 0; x < VGA_WIDTH; x++) {
			vga_buffer[y * VGA_WIDTH + x] = (panic_bg << 8) | ' ';
		}
	}

	set_cursor(2, 1);
	print_color("-------------------------", panic_bg);
	set_cursor(2, 2);
	print_color("[ ", panic_bg);
	print_color(NAME, panic_bg);
	print_color(" - KERNEL PANIC ]", panic_bg);
	set_cursor(2, 3);
	print_color("-------------------------", panic_bg);
	set_cursor(2, 4);
	print_color("Reason: ", panic_bg);
	print_color(message, panic_bg);
	set_cursor(2, 5);
	print_color("HOST: ", panic_bg);
	print_color(HOST, panic_bg);
	set_cursor(2, 6);
	print_color("CPU Arch: ", panic_bg);
	print_color(ARCH, panic_bg);
	set_cursor(2, 7);
	print_color("OS Version: ", panic_bg);
	print_color(VERSION, panic_bg);
	set_cursor(2, 24);
	print_color("System halted. Please restart manually.", panic_footer);

	keyboard_flush();
	while (1) {
		__asm__ volatile("hlt");
	}
}
