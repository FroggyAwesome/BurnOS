#ifndef VFS_H
#define VFS_H

struct file {
	const char *name;
	const char *content;
};

void vfs_init(void);
void vfs_dir(void);
void vfs_cat(const char *filename);

#endif
