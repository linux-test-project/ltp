
#include <linux/fs.h>

struct file;

struct seq_file {
};

struct seq_operations {
	void * (*start) (struct seq_file *m, loff_t *pos);
	void (*stop) (struct seq_file *m, void *v);
	void * (*next) (struct seq_file *m, void *v, loff_t *pos);
	int (*show) (struct seq_file *m, void *v);
};

#define seq_printf(a, b, c...) printf(b , ## c)
static inline int seq_open(struct file *f, const struct seq_operations *o) { return -1; }

static inline ssize_t seq_read(struct file *a, char  *b, size_t c, loff_t *d) { return 0; }
static inline ssize_t seq_write(struct file *a, const char  *b, size_t c, loff_t *d) { return 0; }
static inline int seq_release(struct inode *a, struct file *b) { return 0; }

