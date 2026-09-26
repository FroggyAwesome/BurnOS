#ifndef VGA_H
#define VGA_H

void clear_screen(void);
void vga_putchar(char c);
void print(const char *str);
void vga_putchar_color(char c, unsigned char color);
void print_color(const char *str, unsigned char color);
void hide_cursor(void);
void show_cursor(void);
unsigned short *get_vga_buffer(void);
int get_cursor_x(void);
int get_cursor_y(void);
void set_cursor(int x, int y);
void vga_set_color(unsigned char bg, unsigned char fg);
void vga_reset_color(void);
void set_cursor_form(char type);

#endif
