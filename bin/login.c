#include "login.h"
#include "general.h"
#include "keyboard.h"
#include "os.h"
#include "timer.h"
#include "vga.h"

static int
streq(const char *s1, const char *s2)
{
	while (*s1 && (*s1 == *s2)) {
		s1++;
		s2++;
	}
	return *(const unsigned char *)s1 - *(const unsigned char *)s2;
}

static void
read_password(char *buf, int max_len)
{
	vga_hide_cursor();
	int i = 0;
	while (i < max_len - 1) {
		char c = scancode_to_ascii(get_scancode());
		if (c == 0)
			continue;
		if (c == '\n') {
			vga_putchar('\n');
			break;
		}
		if (c == '\b') {
			if (i > 0) {
				i--;
				vga_putchar('\b');
			}
			continue;
		}
		if (c >= 32 && c <= 126) {
			buf[i++] = c;
			vga_putchar('*');
		}
	}
	buf[i] = '\0';
}

void
login(void)
{
	char username[64];
	char password[64];

	vga_print("The username and password are included when the program is compiled and cannot\nbe modified during execution.\n\n");

	while (1) {
		vga_set_cursor_form('b');
		vga_print(HOST " login: ");
		read_line(username, sizeof(username));

		vga_print("Password: ");
		read_password(password, sizeof(password));

		if (streq(username, USER) == 0 && streq(password, PASSWD) == 0) {
			vga_print("\n");
			break;
		} else {
			vga_print("Login incorrect\n\n");
			sleep_ms(500);
		}
	}
}
