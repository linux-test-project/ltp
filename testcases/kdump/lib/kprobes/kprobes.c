#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uio.h>
#include <linux/kprobes.h>

/*
 * Jumper probe for do_fork.
 * Mirror principle enables access to arguments of the probed routine
 * from the probe handler.
 */

/* Proxy routine having the same arguments as actual do_fork() routine */
long jdo_fork(unsigned long clone_flags, unsigned long stack_start,
	      struct pt_regs *regs, unsigned long stack_size,
	      int __user * parent_tidptr, int __user * child_tidptr)
{
	printk("jprobe: clone_flags=0x%lx, stack_size=0x%lx, regs=%p\n",
	       clone_flags, stack_size, regs);
	/* Always end with a call to jprobe_return(). */
	jprobe_return();

	return 0;
}

static struct jprobe my_jprobe = {
	.entry = jdo_fork
};

static int __init jprobe_init(void)
{
	int ret;
	my_jprobe.kp.symbol_name = "do_fork";

	if ((ret = register_jprobe(&my_jprobe)) < 0) {
		printk("register_jprobe failed, returned %d\n", ret);
		/* XXX: Exit code is wrong. */
		return -1;
	}
	printk("Planted jprobe at %p, handler addr %p\n",
	       my_jprobe.kp.addr, my_jprobe.entry);
	return 0;
}

static void __exit jprobe_exit(void)
{
	unregister_jprobe(&my_jprobe);
	printk("jprobe unregistered\n");
}

module_init(jprobe_init)
    module_exit(jprobe_exit)
    MODULE_LICENSE("GPL");
