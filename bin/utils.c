#include "utils.h"
#include "general.h"
#include "os.h"
#include "rtc.h"
#include "shell.h"
#include "timer.h"
#include "vfs.h"
#include "vga.h"

int
strprefix(const char *pre, const char *str)
{
	while (*pre) {
		if (*pre++ != *str++)
			return 0;
	}
	return 1;
}

static int
parse_int(const char *str)
{
	int res = 0;
	while (*str >= '0' && *str <= '9') {
		res = res * 10 + (*str - '0');
		str++;
	}
	return res;
}

static void
print_int(int n)
{
	if (n == 0) {
		vga_print("0");
		return;
	}
	if (n < 0) {
		vga_print("-");
		n = -n;
	}
	char buf[12];
	int i = 0;
	while (n > 0 && i < 11) {
		buf[i++] = '0' + (n % 10);
		n /= 10;
	}
	char rev[12];
	int r = 0;
	while (i > 0) {
		rev[r++] = buf[--i];
	}
	rev[r] = '\0';
	vga_print(rev);
}

void
cmd_help(void)
{
	vga_print("Available commands:\n");
	vga_print("  SYSTEM   IO   UTILS   CONFIG     HARDWARE SHELL\n");
	vga_print("  -------- ---- ------- ---------- -------- -----\n");
	vga_print("  help     dir  time    set prompt cpu      exit\n");
	vga_print("  whoami   cat  date    set color  gpu      #\n");
	vga_print("  hostname echo day     set cursor colors   {}\n");
	vga_print("  version  cls  uptime\n");
	vga_print("  arch          sleep\n");
	vga_print("  reboot        calc\n");
	vga_print("  poweroff      desktop\n");
	vga_print("  panic\n");
	vga_print("  halt\n");
}

void
cmd_time(void)
{
	int h, m, s;
	read_rtc_time(&h, &m, &s);

	char buf[9];
	buf[0] = '0' + (h / 10);
	buf[1] = '0' + (h % 10);
	buf[2] = ':';
	buf[3] = '0' + (m / 10);
	buf[4] = '0' + (m % 10);
	buf[5] = ':';
	buf[6] = '0' + (s / 10);
	buf[7] = '0' + (s % 10);
	buf[8] = '\0';

	vga_print(buf);
	vga_print("\n");
}

void
cmd_date(void)
{
	int d, m, y;
	read_rtc_date(&d, &m, &y);

	int yy = y % 100;
	char buf[9];
	buf[0] = '0' + (m / 10);
	buf[1] = '0' + (m % 10);
	buf[2] = '/';
	buf[3] = '0' + (d / 10);
	buf[4] = '0' + (d % 10);
	buf[5] = '/';
	buf[6] = '0' + (yy / 10);
	buf[7] = '0' + (yy % 10);
	buf[8] = '\0';

	vga_print(buf);
	vga_print("\n");
}

void
cmd_day(void)
{
	cmd_date();
	cmd_time();
}

void
cmd_sleep(const char *args)
{
	while (*args == ' ')
		args++;
	if (*args == '\0') {
		vga_print("Usage: sleep [SECONDS]\n");
		return;
	}
	int seconds = parse_int(args);
	if (seconds <= 0) {
		vga_print("Error: Invalid duration.\n");
		return;
	}
	sleep((unsigned int)seconds);
}

void
cmd_uptime(void)
{
	unsigned int secs = get_ticks() / 1000;
	unsigned int m = secs / 60;
	unsigned int s = secs % 60;

	char buf[16];
	int i = 0;

	buf[i++] = '0' + (m / 10);
	buf[i++] = '0' + (m % 10);
	buf[i++] = 'm';
	buf[i++] = ' ';
	buf[i++] = '0' + (s / 10);
	buf[i++] = '0' + (s % 10);
	buf[i++] = 's';
	buf[i++] = '\n';
	buf[i] = '\0';

	vga_print("Uptime: ");
	vga_print(buf);
}

void
cmd_colors(void)
{
	vga_print("VGA Color Palette:\n");
	vga_print("  0: Black        8: Dark Grey\n");
	vga_print("  1: Blue         9: Light Blue\n");
	vga_print("  2: Green       10: Light Green\n");
	vga_print("  3: Cyan        11: Light Cyan\n");
	vga_print("  4: Red         12: Light Red\n");
	vga_print("  5: Magenta     13: Light Magenta\n");
	vga_print("  6: Brown       14: Yellow\n");
	vga_print("  7: Light Grey  15: White\n");
}

void
cmd_set(const char *args)
{
	while (*args == ' ')
		args++;

	if (*args == '\0') {
		vga_print("Usage: set [color|prompt|cursor]\n");
		return;
	}

	if (strprefix("color", args)) {
		const char *p = args + 5;
		while (*p == ' ')
			p++;
		if (*p == '\0') {
			vga_print("Usage: set color [BG FG|d]\n");
			return;
		}

		if (p[0] == 'd' && (p[1] == '\0' || p[1] == ' ')) {
			vga_set_color(1, 15);
			vga_clear_screen();
			vga_print("Color reset to default.\n");
			return;
		}

		if (p[0] < '0' || p[0] > '7' || p[1] != ' ') {
			vga_print("Error: Background must be 0-7.\n");
			return;
		}

		int bg = p[0] - '0';
		p += 2;

		if (*p < '0' || *p > '9') {
			vga_print("Error: Foreground must be 0-15.\n");
			return;
		}

		int fg = 0;
		while (*p >= '0' && *p <= '9') {
			fg = fg * 10 + (*p - '0');
			p++;
		}

		if (fg > 15) {
			vga_print("Error: Foreground must be 0-15.\n");
			return;
		}

		vga_set_color((unsigned char)bg, (unsigned char)fg);
		vga_clear_screen();
		vga_print("Color updated successfully.\n");
	} else if (strprefix("prompt", args)) {
		const char *p = args + 6;
		while (*p == ' ')
			p++;
		if (*p == '\0') {
			vga_print("Usage: set prompt [n|c|f]\n");
			return;
		}
		char type = *p;
		shell_set_prompt(type);
	} else if (strprefix("cursor", args)) {
		const char *p = args + 6;
		while (*p == ' ')
			p++;
		if (*p == '\0') {
			vga_print("Usage: set cursor [b|h|l]\n");
			return;
		}
		char type = *p;
		if (type == 'b' || type == 'h' || type == 'l') {
			vga_set_cursor_form(type);
		} else {
			vga_print("Error: Invalid cursor type. Use: b (block), h (half), l (line)\n");
		}
	} else {
		vga_print("Usage: set [color|prompt|cursor]\n");
	}
}

void
cmd_dir(void)
{
	vfs_dir();
}

void
cmd_cat(const char *args)
{
	while (*args == ' ')
		args++;
	if (*args == '\0') {
		vga_print("Usage: cat [FILE]\n");
		return;
	}
	vfs_cat(args);
}

void
cmd_write(const char *args)
{
	while (*args == ' ')
		args++;

	if (*args != '"') {
		vga_print("Usage: write \"filename\" \"content\"\n");
		return;
	}
	args++;

	char filename[32];
	int i = 0;
	while (*args != '"' && *args != '\0' && i < 31) {
		filename[i++] = *args++;
	}
	filename[i] = '\0';

	if (*args != '"') {
		vga_print("Error: Missing closing quote for filename.\n");
		return;
	}
	args++;

	while (*args == ' ')
		args++;

	if (*args != '"') {
		vga_print("Usage: write \"filename\" \"content\"\n");
		return;
	}
	args++;

	char content[128];
	i = 0;
	while (*args != '"' && *args != '\0' && i < 127) {
		content[i++] = *args++;
	}
	content[i] = '\0';

	if (*args != '"') {
		vga_print("Error: Missing closing quote for content.\n");
		return;
	}

	vfs_write(filename, content);
}

void
cmd_rm(const char *args)
{
	while (*args == ' ')
		args++;
	if (*args == '\0') {
		vga_print("Usage: rm [FILE]\n");
		return;
	}
	vfs_rm(args);
}

void
cmd_echo(const char *args)
{
	while (*args == ' ')
		args++;
	if (*args == '\0') {
		vga_print("Usage: echo [TEXT]\n");
		return;
	}
	vga_print(args);
	vga_print("\n");
}

void
cmd_whoami(void)
{
	vga_print(USER "\n");
}

void
cmd_hostname(void)
{
	vga_print(HOST "\n");
}

void
cmd_arch(void)
{
	vga_print(ARCH "\n");
}

void
cmd_version(void)
{
	vga_print(VERSION "\n");
}

void
cmd_calc(const char *args)
{
	while (*args == ' ')
		args++;
	if (*args == '\0') {
		vga_print("Usage: calc [NUM] [+|-|*|/] [NUM]\n");
		return;
	}

	int a = 0;
	int sign1 = 1;
	if (*args == '-') {
		sign1 = -1;
		args++;
	} else if (*args == '+') {
		args++;
	}

	while (*args >= '0' && *args <= '9') {
		a = a * 10 + (*args - '0');
		args++;
	}
	a *= sign1;

	while (*args == ' ')
		args++;

	char op = *args;
	if (op != '+' && op != '-' && op != '*' && op != '/') {
		vga_print("Error: Invalid operator. Use +, -, *, /\n");
		return;
	}
	args++;

	while (*args == ' ')
		args++;

	int b = 0;
	int sign2 = 1;
	if (*args == '-') {
		sign2 = -1;
		args++;
	} else if (*args == '+') {
		args++;
	}

	while (*args >= '0' && *args <= '9') {
		b = b * 10 + (*args - '0');
		args++;
	}
	b *= sign2;

	int res = 0;
	if (op == '+') {
		res = a + b;
	} else if (op == '-') {
		res = a - b;
	} else if (op == '*') {
		res = a * b;
	} else if (op == '/') {
		if (b == 0) {
			vga_print("Error: Division by zero.\n");
			return;
		}
		res = a / b;
	}

	print_int(res);
	vga_print("\n");
}
