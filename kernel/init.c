#include "init.h"
#include "idt.h"
#include "io.h"
#include "keyboard.h"
#include "login.h"
#include "pic.h"
#include "shell.h"
#include "timer.h"
#include "vga.h"

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
	vga_clear_screen();
	vga_hide_cursor();

	sleep_ms(800);
	vga_print("* Kernel loaded successfully.\n");
	vga_print("\n");
	sleep_ms(500);
	vga_print("* Login loaded successfully.\n");
	sleep_ms(500);
	vga_print("* Shell loaded successfully.\n");
	vga_print("\n");
	sleep_ms(800);
	vga_print("Starting...\n");
	vga_print("\n");

	sleep_ms(500);

	keyboard_flush();
	vga_show_cursor();

	while (1) {
		login();
		shell();
	}

	while (1) {
		__asm__ volatile("hlt");
	}
}
