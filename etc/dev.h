#ifndef DEV_H
#define DEV_H

#include "vfs.h"

static struct file files[] = {
	{ "kernel.c", "void main() {\n    // BurnOS kernel core\n}" },
	{ "welcome.txt", "Welcome to BurnOS! Flat file system active." }
};

static const int file_count = 2;

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
#define BUFFER_SIZE 256

#endif
