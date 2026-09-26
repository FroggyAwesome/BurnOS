#include "init.h"
#include "dev.h"
#include "idt.h"
#include "io.h"
#include "kernel.h"
#include "keyboard.h"
#include "login.h"
#include "pic.h"
#include "shell.h"
#include "timer.h"
#include "vga.h"

static void
print_init_step(const char *tag, unsigned char tag_color, const char *msg)
{
	print_color("[ ", TAG);
	print_color(tag, tag_color);
	print_color(" ] ", TAG);
	print_color(msg, MSG);
	print_color("\n", DEFAULT_COLOR);
}

void
init(void)
{
	pic_remap(32, 40);
	outb(0x21, 0xFC);
	outb(0xA1, 0xFF);
	idt_init();
	init_pit(1000);
	__asm__ volatile("sti");
	vga_set_color(INIT_BG, VGA_FG_WHITE);
	clear_screen();
	hide_cursor();

	sleep_ms(500);
	print_init_step("OK", OK, "Kernel Core Initialized.");
	sleep_ms(500);
	print_init_step("OK", OK, "Initialized VGA Text Mode Buffer (80x25).");
	sleep_ms(500);
	print_init_step("OK", OK, "Loaded Low-Level I/O Port Drivers.");
	sleep_ms(500);
	print_init_step("OK", OK, "Loaded PS/2 Keyboard Driver & Scancodes.");
	sleep_ms(500);
	print_init_step("OK", OK, "Mounted Virtual File System (VFS).");
	sleep_ms(250);
	print_init_step("OK", OK, "Started BurnOS Command Shell.");
	sleep_ms(500);
	print_init_step("OK", OK, "Loaded Authentication & Login Module.");
	print_init_step("OK", OK, "System Ready...");
	print("\n");

	sleep_ms(500);

	keyboard_flush();
	show_cursor();

	while (1) {
		login();
		shell();
	}

	while (1) {
		__asm__ volatile("hlt");
	}
}
