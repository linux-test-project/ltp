struct inode;
struct file;
struct dentry;

struct file_operations {
	ssize_t (*read) (struct file *, char  *, size_t, loff_t *);
	int (*open) (struct inode *, struct file *);
	int (*release) (struct inode *, struct file *);
	ssize_t (*write) (struct file *, const char *, size_t, loff_t *);
};
