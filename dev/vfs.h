#ifndef VFS_H
#define VFS_H

#define MAX_FILES 8

struct file {
	char name[32];
	char content[128];
};

void vfs_init(void);
void vfs_dir(void);
void vfs_cat(const char *filename);
void vfs_write(const char *filename, const char *content);
void vfs_rm(const char *filename);

#endif
