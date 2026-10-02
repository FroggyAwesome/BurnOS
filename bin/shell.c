#include "shell.h"
#include "desktop.h"
#include "general.h"
#include "keyboard.h"
#include "system.h"
#include "utils.h"
#include "vga.h"

static char current_prompt[32] = DEFAULT_PROMPT;

static int
streq(const char *s1, const char *s2)
{
	while (*s1 && (*s1 == *s2)) {
		s1++;
		s2++;
	}
	return *(const unsigned char *)s1 - *(const unsigned char *)s2;
}

void
shell_set_prompt(char type)
{
	const char *src = SH_PROMPT_NORMAL;

	if (type == 'n') {
		src = SH_PROMPT_NORMAL;
	} else if (type == 'c') {
		src = SH_PROMPT_CLASSIC;
	} else if (type == 'f') {
		src = SH_PROMPT_FIXED;
	} else {
		vga_print("Invalid prompt type. Use: n, c, f\n");
		return;
	}

	int i = 0;
	while (src[i] != '\0' && i < 31) {
		current_prompt[i] = src[i];
		i++;
	}
	current_prompt[i] = '\0';
}

static void
execute_command(const char *buffer)
{
	if (streq(buffer, "help") == 0) {
		cmd_help();
	} else if (streq(buffer, "dir") == 0) {
		cmd_dir();
	} else if (strprefix("cat", buffer)) {
		cmd_cat(buffer + (buffer[3] == ' ' ? 4 : 3));
	} else if (strprefix("write", buffer)) {
		cmd_write(buffer + (buffer[5] == ' ' ? 6 : 5));
	} else if (strprefix("rm", buffer)) {
		cmd_rm(buffer + (buffer[2] == ' ' ? 3 : 2));
	} else if (streq(buffer, "poweroff") == 0) {
		poweroff();
	} else if (streq(buffer, "reboot") == 0) {
		reboot();
	} else if (streq(buffer, "cls") == 0) {
		vga_clear_screen();
	} else if (streq(buffer, "version") == 0) {
		cmd_version();
	} else if (streq(buffer, "arch") == 0) {
		cmd_arch();
	} else if (strprefix("echo", buffer)) {
		cmd_echo(buffer + (buffer[4] == ' ' ? 5 : 4));
	} else if (streq(buffer, "whoami") == 0) {
		cmd_whoami();
	} else if (streq(buffer, "hostname") == 0) {
		cmd_hostname();
	} else if (streq(buffer, "panic") == 0) {
		panic("Manually executed by the user.");
	} else if (strprefix("panic ", buffer)) {
		panic(buffer + 6);
	} else if (streq(buffer, "halt") == 0) {
		halt();
	} else if (strprefix("set", buffer)) {
		cmd_set(buffer + (buffer[3] == ' ' ? 4 : 3));
	} else if (streq(buffer, "time") == 0) {
		cmd_time();
	} else if (streq(buffer, "date") == 0) {
		cmd_date();
	} else if (streq(buffer, "day") == 0) {
		cmd_day();
	} else if (strprefix("sleep", buffer)) {
		cmd_sleep(buffer + (buffer[5] == ' ' ? 6 : 5));
	} else if (streq(buffer, "uptime") == 0) {
		cmd_uptime();
	} else if (streq(buffer, "colors") == 0) {
		cmd_colors();
	} else if (streq(buffer, "cpu") == 0) {
		cpu();
	} else if (streq(buffer, "gpu") == 0) {
		gpu();
	} else if (streq(buffer, "desktop") == 0) {
		desktop();
	} else if (strprefix("calc", buffer)) {
		cmd_calc(buffer + (buffer[4] == ' ' ? 5 : 4));
	} else {
		vga_print("Command not found. Type 'help' for available commands.\n");
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
	vga_clear_screen();
	vga_print(MOTD);
	vga_set_cursor_form('b');

	while (1) {
		if (!in_block) {
			vga_print(current_prompt);
		} else {
			vga_print("> ");
		}

		read_line(buffer, sizeof(buffer));

		if (buffer[0] == '#' || (buffer[0] == '\0' && !in_block)) {
			continue;
		}

		if (!in_block && streq(buffer, "exit") == 0) {
			vga_reset_color();
			vga_clear_screen();
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
