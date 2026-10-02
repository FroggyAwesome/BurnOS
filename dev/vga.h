#ifndef VGA_H
#define VGA_H

#define VGA_WIDTH 80
#define VGA_HEIGHT 25
#define VGA_BUFFER 0xB8000

#define VGA_BG_BLACK 0
#define VGA_BG_BLUE 1
#define VGA_BG_GREEN 2
#define VGA_BG_CYAN 3
#define VGA_BG_RED 4
#define VGA_BG_MAGENTA 5
#define VGA_BG_BROWN 6
#define VGA_BG_LIGHT_GREY 7

#define VGA_FG_BLACK 0
#define VGA_FG_BLUE 1
#define VGA_FG_GREEN 2
#define VGA_FG_CYAN 3
#define VGA_FG_RED 4
#define VGA_FG_MAGENTA 5
#define VGA_FG_BROWN 6
#define VGA_FG_LIGHT_GREY 7
#define VGA_FG_DARK_GREY 8
#define VGA_FG_LIGHT_BLUE 9
#define VGA_FG_LIGHT_GREEN 10
#define VGA_FG_LIGHT_CYAN 11
#define VGA_FG_LIGHT_RED 12
#define VGA_FG_LIGHT_MAGENTA 13
#define VGA_FG_YELLOW 14
#define VGA_FG_WHITE 15

#define VGA_ENTRY_COLOR(bg, fg) ((unsigned char)(((bg & 0x07) << 4) | ((fg) & 0x0F)))
#define DEFAULT_COLOR VGA_ENTRY_COLOR(VGA_BG_BLACK, VGA_FG_WHITE)

void vga_clear_screen(void);
void vga_putchar(char c);
void vga_print(const char *str);
void vga_putchar_color(char c, unsigned char color);
void vga_print_color(const char *str, unsigned char color);
void vga_hide_cursor(void);
void vga_show_cursor(void);
unsigned short *vga_get_buffer(void);
int vga_get_cursor_x(void);
int vga_get_cursor_y(void);
void vga_set_cursor(int x, int y);
void vga_set_color(unsigned char bg, unsigned char fg);
void vga_reset_color(void);
void vga_set_cursor_form(char type);
void vga_putchar_at(int x, int y, char c, unsigned char color);

#endif
