#include "vfs.h"
#include "vga.h"

static struct file files[MAX_FILES] = {
	{ "kernel.c", "void main() {\n    // BurnOS kernel core\n}" },
	{ "welcome.txt", "Welcome to BurnOS! Flat file system active." }
};

static int file_count = 2;

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
str_copy(char *dest, const char *src, int max_len)
{
	int i = 0;
	while (src[i] != '\0' && i < max_len - 1) {
		dest[i] = src[i];
		i++;
	}
	dest[i] = '\0';
}

void
vfs_init(void)
{
}

void
vfs_dir(void)
{
	for (int i = 0; i < file_count; i++) {
		vga_print(files[i].name);
		if (i < file_count - 1) {
			vga_print(", ");
		}
	}
	vga_print("\n");
}

void
vfs_cat(const char *filename)
{
	for (int i = 0; i < file_count; i++) {
		if (streq(files[i].name, filename) == 0) {
			vga_print(files[i].content);
			vga_print("\n");
			return;
		}
	}
	vga_print("File not found.\n");
}

void
vfs_write(const char *filename, const char *content)
{
	for (int i = 0; i < file_count; i++) {
		if (streq(files[i].name, filename) == 0) {
			str_copy(files[i].content, content, 128);
			vga_print("File updated successfully.\n");
			return;
		}
	}

	if (file_count >= 8) {
		vga_print("Error: VFS is full.\n");
		return;
	}

	str_copy(files[file_count].name, filename, 32);
	str_copy(files[file_count].content, content, 128);
	file_count++;
	vga_print("File created successfully.\n");
}

void
vfs_rm(const char *filename)
{
	int found = -1;
	for (int i = 0; i < file_count; i++) {
		if (streq(files[i].name, filename) == 0) {
			found = i;
			break;
		}
	}

	if (found == -1) {
		vga_print("File not found.\n");
		return;
	}

	for (int i = found; i < file_count - 1; i++) {
		for (int j = 0; j < 32; j++) {
			files[i].name[j] = files[i + 1].name[j];
		}
		for (int j = 0; j < 128; j++) {
			files[i].content[j] = files[i + 1].content[j];
		}
	}

	file_count--;
	vga_print("File removed successfully.\n");
}
