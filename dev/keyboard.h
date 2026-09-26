#ifndef KEYBOARD_H
#define KEYBOARD_H

unsigned char get_scancode(void);
char scancode_to_ascii(unsigned char scancode);
void read_line(char *buf, int max_len);
void keyboard_flush(void);

#endif
