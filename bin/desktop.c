#include "desktop.h"
#include "keyboard.h"
#include "vga.h"

void
desktop(void)
{
	unsigned char desktop_color = VGA_ENTRY_COLOR(VGA_BG_BLUE, VGA_FG_WHITE);
	unsigned char taskbar_color = VGA_ENTRY_COLOR(VGA_BG_GREEN, VGA_FG_WHITE);
	unsigned char start_btn_color = VGA_ENTRY_COLOR(VGA_BG_CYAN, VGA_FG_BLACK);

	vga_hide_cursor();

	for (int y = 0; y < 24; y++) {
		for (int x = 0; x < VGA_WIDTH; x++) {
			vga_putchar_at(x, y, ' ', desktop_color);
		}
	}

	for (int x = 0; x < VGA_WIDTH; x++) {
		vga_putchar_at(x, 24, ' ', taskbar_color);
	}

	const char *start_text = " Start ";
	for (int i = 0; start_text[i] != '\0'; i++) {
		vga_putchar_at(i, 24, start_text[i], start_btn_color);
	}

	while (1) {
		char c = scancode_to_ascii(get_scancode());
		if (c == 27) {
			vga_show_cursor();
			break;
		}
	}

	vga_clear_screen();
}
