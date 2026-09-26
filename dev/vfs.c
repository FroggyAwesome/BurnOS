#include "vfs.h"
#include "dev.h"
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

void
vfs_init(void)
{
}

void
vfs_dir(void)
{
	for (int i = 0; i < file_count; i++) {
		print(files[i].name);
		if (i < file_count - 1) {
			print(", ");
		}
	}
	print("\n");
}

void
vfs_cat(const char *filename)
{
	for (int i = 0; i < file_count; i++) {
		if (streq(files[i].name, filename) == 0) {
			print(files[i].content);
			print("\n");
			return;
		}
	}
	print("File not found.\n");
}
