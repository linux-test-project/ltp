/*
 * Crasher module for kdump testing
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 * Copyright © IBM Corporation 2007
 *
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/proc_fs.h>
#include <linux/spinlock.h>
#include <asm/uaccess.h>
#include <linux/version.h>
#include <linux/uaccess.h>

MODULE_LICENSE("GPL");

int crasher_init(void);
void crasher_exit(void);
module_init(crasher_init);
module_exit(crasher_exit);

#define CRASH "crasher"		/* name of /proc entry file */

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,32)
static int crasher_read(char *buf, char **start, off_t offset, int len,
			int *eof, void *data)
#else
static ssize_t crasher_read(struct file *file, char __user *buf, size_t len,
				loff_t *offset)
#endif
{
	return (sprintf(buf, "\n"));
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,32)
static int crasher_write(struct file *file, const char *buffer,
			 unsigned long count, void *data)
#else
static ssize_t crasher_write(struct file *file, const char __user *buffer,
			size_t count, loff_t *data)
#endif
{
	char value, *a;
	DEFINE_SPINLOCK(mylock);

	/* grab the first byte the user gave us, ignore the rest */
	if (copy_from_user(&value, buffer, 1))
		return -EFAULT;

	switch (value) {
	case '0':		/* panic the system */
		panic("KDUMP test panic\n");
		break;

	case '1':		/* BUG() test */
		BUG();
		break;

	case '2':		/* panic_on_oops test */
		a = 0;
		a[1] = 'A';
		break;

	case '3':		/* hang w/double spinlock */
		spin_lock_irq(&mylock);
		spin_lock_irq(&mylock);
		break;

	default:
		printk("crasher: Bad command\n");
	}

	return count;		/* tell the user we read all his data,
				   somtimes white lies are ok */
}

/* create a directory in /proc and a debug file in the new directory */

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,32)
static const struct file_operations crasher_proc_fops = {
	.owner = THIS_MODULE,
	.read  = crasher_read,
	.write = crasher_write,
};
#endif

int crasher_init(void)
{
	struct proc_dir_entry *crasher_proc;

	printk("loaded crasher module\n");

	/* build a crasher file that can be set */
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,32)
	if ((crasher_proc = create_proc_entry(CRASH, 0, NULL)) == NULL) {
		return -ENOMEM;
	}
	crasher_proc->owner = THIS_MODULE
	crasher_proc->read_proc = crasher_read;
	crasher_proc->write_proc = crasher_write;
#else
	crasher_proc = proc_create_data(CRASH, 0, NULL,
	                                &crasher_proc_fops, NULL);
	if (!crasher_proc)
		return -ENOMEM;
#endif
	return 0;
}

void crasher_exit(void)
{
	remove_proc_entry(CRASH, NULL);
	printk("removed crasher module\n");
}
