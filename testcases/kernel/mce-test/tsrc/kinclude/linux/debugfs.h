
static inline struct dentry *debugfs_create_file(const char *name, mode_t mode,
				   struct dentry *parent, void *data,
				   const struct file_operations *fops)
{
	return NULL;
}

static inline struct dentry *debugfs_create_dir(const char *name, struct dentry *parent)
{
	return NULL;
}


static inline void debugfs_remove(struct dentry *dentry) { } 
