#include "system.h"
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
	vga_hide_cursor();
	vga_print("System halted. Kernel stopped.\n");
	__asm__ volatile("cli");
	while (1) {
		__asm__ volatile("hlt");
	}
}

void
panic(const char *message)
{
	__asm__ volatile("cli");
	vga_hide_cursor();

	unsigned short *vga_buffer = vga_get_buffer();
	unsigned char panic_bg = VGA_ENTRY_COLOR(VGA_BG_RED, VGA_FG_WHITE);
	unsigned char panic_footer = VGA_ENTRY_COLOR(VGA_BG_RED, VGA_FG_BLACK);

	for (int y = 0; y < VGA_HEIGHT; y++) {
		for (int x = 0; x < VGA_WIDTH; x++) {
			vga_buffer[y * VGA_WIDTH + x] = (panic_bg << 8) | ' ';
		}
	}

	vga_set_cursor(2, 1);
	vga_print_color("-------------------------", panic_bg);
	vga_set_cursor(2, 2);
	vga_print_color("[ ", panic_bg);
	vga_print_color(NAME, panic_bg);
	vga_print_color(" - KERNEL PANIC ]", panic_bg);
	vga_set_cursor(2, 3);
	vga_print_color("-------------------------", panic_bg);
	vga_set_cursor(2, 4);
	vga_print_color("Reason: ", panic_bg);
	vga_print_color(message, panic_bg);
	vga_set_cursor(2, 5);
	vga_print_color("HOST: ", panic_bg);
	vga_print_color(HOST, panic_bg);
	vga_set_cursor(2, 6);
	vga_print_color("Arch: ", panic_bg);
	vga_print_color(ARCH, panic_bg);
	vga_set_cursor(2, 7);
	vga_print_color("Version: ", panic_bg);
	vga_print_color(VERSION, panic_bg);
	vga_set_cursor(2, 24);
	vga_print_color("System halted. Please restart manually.", panic_footer);

	keyboard_flush();
	while (1) {
		__asm__ volatile("hlt");
	}
}

void
cpu(void)
{
	unsigned int ebx, edx, ecx;
	__asm__ volatile(
			"cpuid"
			: "=b"(ebx), "=d"(edx), "=c"(ecx)
			: "a"(0));

	char vendor[13];
	*(unsigned int *)&vendor[0] = ebx;
	*(unsigned int *)&vendor[4] = edx;
	*(unsigned int *)&vendor[8] = ecx;
	vendor[12] = '\0';

	vga_print("CPU Vendor: ");
	vga_print(vendor);
	vga_print("\n");
}

void
gpu(void)
{
	vga_print("GPU / Display Info:\n");
	vga_print("  Controller: VGA Text Mode\n");
	vga_print("  Resolution: 80x25 text cells\n");
	vga_print("  Memory:     0xB8000\n");
}
