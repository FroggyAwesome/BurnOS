#include "shell.h"

#include "keyboard.h"
#include "rtc.h"
#include "timer.h"
#include "vfs.h"
#include "vga.h"

#include "bin.h"
#include "dev.h"
#include "general.h"
#include "os.h"

#include "system.h"

static char current_prompt[32] = DEFAULT_PROMPT;
static unsigned char current_bg = 1;
static unsigned char current_fg = 15;

static int
streq(const char *s1, const char *s2)
{
	while (*s1 && (*s1 == *s2)) {
		s1++;
		s2++;
	}
	return *(const unsigned char *)s1 - *(const unsigned char *)s2;
}

static int
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
execute_set(const char *args)
{
	if (strprefix("color ", args)) {
		const char *p = args + 6;

		if (p[0] == 'd' && (p[1] == '\0' || p[1] == ' ')) {
			vga_set_color(SHELL_BG, SHELL_FG);
			clear_screen();
			print("Color reset to default.\n");
			return;
		}

		if (p[0] < '0' || p[0] > '7' || p[1] != ' ') {
			print("Error: Background must be 0-7.\n");
			return;
		}

		int bg = p[0] - '0';
		p += 2;

		if (*p < '0' || *p > '9') {
			print("Error: Foreground must be 0-15.\n");
			return;
		}

		int fg = 0;
		while (*p >= '0' && *p <= '9') {
			fg = fg * 10 + (*p - '0');
			p++;
		}

		if (fg > 15) {
			print("Error: Foreground must be 0-15.\n");
			return;
		}

		current_bg = (unsigned char)bg;
		current_fg = (unsigned char)fg;

		vga_set_color(current_bg, current_fg);
		clear_screen();
		print("Color updated successfully.\n");
	} else if (strprefix("prompt ", args)) {
		char type = args[7];
		const char *src = SH_PROMPT_NORMAL;

		if (type == 'n') {
			src = SH_PROMPT_NORMAL;
		} else if (type == 'c') {
			src = SH_PROMPT_CLASSIC;
		} else if (type == 'f') {
			src = SH_PROMPT_FIXED;
		} else {
			print("Invalid prompt type. Use: n, c, f\n");
			return;
		}

		int i = 0;
		while (src[i] != '\0' && i < 31) {
			current_prompt[i] = src[i];
			i++;
		}
		current_prompt[i] = '\0';
	} else if (strprefix("cursor ", args)) {
		char type = args[7];
		if (type == 'b' || type == 'h' || type == 'l') {
			set_cursor_form(type);
		} else {
			print("Invalid cursor type. Use: b (block), h (half), l (line)\n");
		}
	} else {
		print("Invalid set syntax. Use 'set color', 'set prompt', or 'set cursor'\n");
	}
}

static void
execute_time(void)
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

	print(buf);
	print("\n");
}

static void
execute_date(void)
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

	print(buf);
	print("\n");
}

static void
execute_day(void)
{
	execute_date();
	execute_time();
}

static void
execute_sleep(const char *args)
{
	int seconds = parse_int(args);
	if (seconds <= 0) {
		print("Error: Invalid duration.\n");
		return;
	}
	sleep((unsigned int)seconds);
}

static void
execute_uptime(void)
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

	print("Uptime: ");
	print(buf);
}

static void
execute_colors(void)
{
	print("VGA Color Palette:\n");
	print("  0: Black        8: Dark Grey\n");
	print("  1: Blue         9: Light Blue\n");
	print("  2: Green       10: Light Green\n");
	print("  3: Cyan        11: Light Cyan\n");
	print("  4: Red         12: Light Red\n");
	print("  5: Magenta     13: Light Magenta\n");
	print("  6: Brown       14: Yellow\n");
	print("  7: Light Grey  15: White\n");
}

static void
execute_cpu(void)
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

	print("CPU Vendor: ");
	print(vendor);
	print("\n");
}

static void
execute_gpu(void)
{
	print("GPU / Display Info:\n");
	print("  Controller: VGA Text Mode\n");
	print("  Resolution: 80x25 text cells\n");
	print("  Memory:     0xB8000\n");
}

static void
execute_help(void)
{
	print("Available commands:\n");
	print("  SYSTEM         IO & UTILS     CONFIG & HARDWARE\n");
	print("  -------------- -------------- -------------------\n");
	print("  help           dir            set prompt [n|c|f]\n");
	print("  whoami         cat [FILE]     set color [BG FG|d]\n");
	print("  hostname       echo [TEXT]    set cursor [b|h|l]\n");
	print("  version        cls            sleep [X]\n");
	print("  arch           time           colors\n");
	print("  reboot         date           cpu\n");
	print("  poweroff       day            gpu\n");
	print("  exit           uptime         # [COM]\n");
	print("  panic [MSG]                   {}\n");
	print("  halt\n");
}

static void
execute_command(const char *buffer)
{
	if (streq(buffer, "help") == 0) {
		execute_help();
	} else if (streq(buffer, "dir") == 0) {
		vfs_dir();
	} else if (strprefix("cat ", buffer)) {
		vfs_cat(buffer + 4);
	} else if (streq(buffer, "poweroff") == 0) {
		poweroff();
	} else if (streq(buffer, "reboot") == 0) {
		reboot();
	} else if (streq(buffer, "cls") == 0) {
		clear_screen();
	} else if (streq(buffer, "version") == 0) {
		print(VERSION "\n");
	} else if (streq(buffer, "arch") == 0) {
		print(ARCH "\n");
	} else if (strprefix("echo ", buffer)) {
		print(buffer + 5);
		print("\n");
	} else if (streq(buffer, "whoami") == 0) {
		print(USER "\n");
	} else if (streq(buffer, "hostname") == 0) {
		print(HOST "\n");
	} else if (streq(buffer, "panic") == 0) {
		panic("Manually executed by the user.");
	} else if (strprefix("panic ", buffer)) {
		panic(buffer + 6);
	} else if (streq(buffer, "halt") == 0) {
		halt();
	} else if (strprefix("set ", buffer)) {
		execute_set(buffer + 4);
	} else if (streq(buffer, "time") == 0) {
		execute_time();
	} else if (streq(buffer, "date") == 0) {
		execute_date();
	} else if (streq(buffer, "day") == 0) {
		execute_day();
	} else if (strprefix("sleep ", buffer)) {
		execute_sleep(buffer + 6);
	} else if (streq(buffer, "uptime") == 0) {
		execute_uptime();
	} else if (streq(buffer, "colors") == 0) {
		execute_colors();
	} else if (streq(buffer, "cpu") == 0) {
		execute_cpu();
	} else if (streq(buffer, "gpu") == 0) {
		execute_gpu();
	} else {
		print("Command not found. Type 'help' for available commands.\n");
	}
}

void
shell(void)
{
	char buffer[256];
	char block_cmds[16][256];
	int block_count = 0;
	int in_block = 0;

	for (int i = 0; DEFAULT_PROMPT[i] != '\0' && i < 31; i++) {
		current_prompt[i] = DEFAULT_PROMPT[i];
		current_prompt[i + 1] = '\0';
	}

	vga_set_color(SHELL_BG, SHELL_FG);
	clear_screen();
	print(MOTD);
	set_cursor_form('b');

	while (1) {
		if (!in_block) {
			print(current_prompt);
		} else {
			print("> ");
		}

		read_line(buffer, sizeof(buffer));

		if (buffer[0] == '#' || (buffer[0] == '\0' && !in_block)) {
			continue;
		}

		if (!in_block && streq(buffer, "exit") == 0) {
			vga_reset_color();
			clear_screen();
			return;
		}

		if (!in_block && streq(buffer, "{") == 0) {
			in_block = 1;
			block_count = 0;
			continue;
		}

		if (in_block) {
			if (streq(buffer, "}") == 0) {
				in_block = 0;
				for (int i = 0; i < block_count; i++) {
					execute_command(block_cmds[i]);
				}
				block_count = 0;
				continue;
			}

			if (buffer[0] != '\0' && block_count < 16) {
				int j = 0;
				while (buffer[j] != '\0' && j < 255) {
					block_cmds[block_count][j] = buffer[j];
					j++;
				}
				block_cmds[block_count][j] = '\0';
				block_count++;
			}
			continue;
		}

		execute_command(buffer);
	}
}
