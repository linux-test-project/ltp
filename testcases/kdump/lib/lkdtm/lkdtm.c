/*
 * Kprobe module for testing crash dumps
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
 * Copyright (C) IBM Corporation, 2006
 *
 * Author: Ankita Garg <ankita@in.ibm.com>
 *         Sachin Sant <sachinp@in.ibm.com>
 *         Cai Qian <qcai@redhat.com>
 *
 * This module induces system failures at predefined crashpoints to
 * evaluate the reliability of crash dumps obtained using different dumping
 * solutions.
 *
 * It is adapted from the Linux Kernel Dump Test Tool by
 * Fernando Luis Vazquez Cao <http://lkdtt.sourceforge.net>
 *
 * Usage :  insmod lkdtm.ko [recur_count={>0}] cpoint_name=<> cpoint_type=<>
 *							[cpoint_count={>0}]
 *
 * recur_count : Recursion level for the stack overflow test. Default is 10.
 *
 * cpoint_name : Crash point where the kernel is to be crashed. It can be
 *		 one of INT_HARDWARE_ENTRY, INT_HW_IRQ_EN, INT_TASKLET_ENTRY,
 *		 FS_DEVRW, MEM_SWAPOUT, TIMERADD, SCSI_DISPATCH_CMD,
 *		 IDE_CORE_CP
 *
 * cpoint_type : Indicates the action to be taken on hitting the crash point.
 *		 It can be one of PANIC, BUG, EXCEPTION, LOOP, OVERFLOW
 *
 * cpoint_count : Indicates the number of times the crash point is to be hit
 *		  to trigger an action. The default is 10.
 */

#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/module.h>
#include <linux/buffer_head.h>
#include <linux/kprobes.h>
#include <linux/list.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/hrtimer.h>
#include <scsi/scsi_cmnd.h>
#include <linux/version.h>
#include <linux/kallsyms.h>

#ifdef CONFIG_IDE
#include <linux/ide.h>
#endif

#define NUM_CPOINTS 8
#define NUM_CPOINT_TYPES 5
#define DEFAULT_COUNT 10
#define REC_NUM_DEFAULT 10

enum cname {
	INVALID,
	INT_HARDWARE_ENTRY,
	INT_HW_IRQ_EN,
	INT_TASKLET_ENTRY,
	FS_DEVRW,
	MEM_SWAPOUT,
	TIMERADD,
	SCSI_DISPATCH_CMD,
	IDE_CORE_CP
};

enum ctype {
	NONE,
	PANIC,
	BUG,
	EXCEPTION,
	LOOP,
	OVERFLOW
};

static char *cp_name[] = {
	"INT_HARDWARE_ENTRY",
	"INT_HW_IRQ_EN",
	"INT_TASKLET_ENTRY",
	"FS_DEVRW",
	"MEM_SWAPOUT",
	"TIMERADD",
	"SCSI_DISPATCH_CMD",
	"IDE_CORE_CP"
};

static char *cp_type[] = {
	"PANIC",
	"BUG",
	"EXCEPTION",
	"LOOP",
	"OVERFLOW"
};

static struct jprobe lkdtm;

static int lkdtm_parse_commandline(void);
static void lkdtm_handler(void);

static char *cpoint_name = INVALID;
static char *cpoint_type = NONE;
static int cpoint_count = DEFAULT_COUNT;
static int recur_count = REC_NUM_DEFAULT;

static enum cname cpoint = INVALID;
static enum ctype cptype = NONE;
static int count = DEFAULT_COUNT;

module_param(recur_count, int, 0644);
MODULE_PARM_DESC(recur_count, " Recursion level for the stack overflow test, "
		 "default is 10");
module_param(cpoint_name, charp, 0644);
MODULE_PARM_DESC(cpoint_name, " Crash Point, where kernel is to be crashed");
module_param(cpoint_type, charp, 0644);
MODULE_PARM_DESC(cpoint_type, " Crash Point Type, action to be taken on "
		 "hitting the crash point");
module_param(cpoint_count, int, 0644);
MODULE_PARM_DESC(cpoint_count, " Crash Point Count, number of times the "
		 "crash point is to be hit to trigger action");

unsigned int jp_do_irq(unsigned int irq)
{
	lkdtm_handler();
	jprobe_return();
	return 0;
}

irqreturn_t jp_handle_irq_event(unsigned int irq, struct irqaction * action)
{
	lkdtm_handler();
	jprobe_return();
	return 0;
}

void jp_tasklet_action(struct softirq_action *a)
{
	lkdtm_handler();
	jprobe_return();
}

void jp_ll_rw_block(int rw, int nr, struct buffer_head *bhs[])
{
	lkdtm_handler();
	jprobe_return();
}

struct scan_control;

unsigned long jp_shrink_page_list(struct list_head *page_list,
				  struct scan_control *sc)
{
	lkdtm_handler();
	jprobe_return();
	return 0;
}

int jp_hrtimer_start(struct hrtimer *timer, ktime_t tim,
		     const enum hrtimer_mode mode)
{
	lkdtm_handler();
	jprobe_return();
	return 0;
}

int jp_scsi_dispatch_cmd(struct scsi_cmnd *cmd)
{
	lkdtm_handler();
	jprobe_return();
	return 0;
}

#ifdef CONFIG_IDE
int jp_generic_ide_ioctl(ide_drive_t * drive, struct file *file,
			 struct block_device *bdev, unsigned int cmd,
			 unsigned long arg)
{
	lkdtm_handler();
	jprobe_return();
	return 0;
}
#endif

static int lkdtm_parse_commandline(void)
{
	int i;

	if (cpoint_name == INVALID || cpoint_type == NONE ||
	    cpoint_count < 1 || recur_count < 1)
		return -EINVAL;

	for (i = 0; i < NUM_CPOINTS; ++i) {
		if (!strcmp(cpoint_name, cp_name[i])) {
			cpoint = i + 1;
			break;
		}
	}

	for (i = 0; i < NUM_CPOINT_TYPES; ++i) {
		if (!strcmp(cpoint_type, cp_type[i])) {
			cptype = i + 1;
			break;
		}
	}

	if (cpoint == INVALID || cptype == NONE)
		return -EINVAL;

	count = cpoint_count;

	return 0;
}

static int recursive_loop(int a)
{
	char buf[1024];

	memset(buf, 0xFF, 1024);
	recur_count--;
	if (!recur_count)
		return 0;
	else
		return recursive_loop(a);
}

void lkdtm_handler(void)
{
	/* Escape endless loop. */
	if (count < 0)
		return;

	printk(KERN_INFO "lkdtm : Crash point %s of type %s hit\n",
	       cpoint_name, cpoint_type);
	--count;

	if (count == 0) {
		switch (cptype) {
		case NONE:
			break;
		case PANIC:
			printk(KERN_INFO "lkdtm : PANIC\n");
			panic("dumptest");
			break;
		case BUG:
			printk(KERN_INFO "lkdtm : BUG\n");
			BUG();
			break;
		case EXCEPTION:
			printk(KERN_INFO "lkdtm : EXCEPTION\n");
			*((int *)0) = 0;
			break;
		case LOOP:
			printk(KERN_INFO "lkdtm : LOOP\n");
			for (;;) ;
			break;
		case OVERFLOW:
			printk(KERN_INFO "lkdtm : OVERFLOW\n");
			(void)recursive_loop(0);
			break;
		default:
			break;
		}
		count = cpoint_count;
	}
}

#ifdef USE_SYMBOL_NAME
void lkdtm_symbol_name(char *name, void (*entry) (void))
{
	lkdtm.kp.symbol_name = name;
	lkdtm.entry = (kprobe_opcode_t *) entry;
}

#else
void lkdtm_lookup_name(char *name, void (*entry) (void))
{
	unsigned long addr;

	addr = kallsyms_lookup_name(name);
	if (addr) {
		*(lkdtm.kp.addr) = addr;
		lkdtm.entry = JPROBE_ENTRY(entry);
	} else
		printk(KERN_INFO "lkdtm : Crash point not available\n");
}
#endif

int lkdtm_module_init(void)
{
	int ret;

	if (lkdtm_parse_commandline() == -EINVAL) {
		printk(KERN_INFO "lkdtm : Invalid command\n");
		return -EINVAL;
	}

	switch (cpoint) {
	case INT_HARDWARE_ENTRY:

#ifdef USE_SYMBOL_NAME

#ifdef __powerpc__
		lkdtm_symbol_name(".__do_IRQ", (void (*)(void))jp_do_irq);
#else
		lkdtm_symbol_name("__do_IRQ", (void (*)(void))jp_do_irq);
#endif /*__powerpc__*/

#else /* USE_SYMBOL_NAME */
		lkdtm_lookup_name("__do_IRQ", (void (*)(void))jp_do_irq);

#endif /* USE_SYMBOL_NAME */
		break;

	case INT_HW_IRQ_EN:

#ifdef USE_SYMBOL_NAME

#ifdef __powerpc__
		lkdtm_symbol_name(".handle_IRQ_event",
				  (void (*)(void))jp_handle_irq_event);
#else
		lkdtm_symbol_name("handle_IRQ_event",
				  (void (*)(void))jp_handle_irq_event);
#endif /*__powerpc__*/

#else /* USE_SYMBOL_NAME */
		lkdtm_lookup_name("handle_IRQ_event",
				  (void (*)(void))jp_handle_irq_event);

#endif /* USE_SYMBOL_NAME */
		break;

	case INT_TASKLET_ENTRY:

#ifdef USE_SYMBOL_NAME

#ifdef __powerpc__
		lkdtm_symbol_name(".tasklet_action",
				  (void (*)(void))jp_tasklet_action);
#else
		lkdtm_symbol_name("tasklet_action",
				  (void (*)(void))jp_tasklet_action);
#endif /*__powerpc__*/

#else /* USE_SYMBOL_NAME */
		lkdtm_lookup_name("tasklet_action",
				  (void (*)(void))jp_tasklet_action);

#endif /* USE_SYMBOL_NAME */
		break;

	case FS_DEVRW:

#ifdef USE_SYMBOL_NAME

#ifdef __powerpc__
		lkdtm_symbol_name(".ll_rw_block",
				  (void (*)(void))jp_ll_rw_block);
#else
		lkdtm_symbol_name("ll_rw_block",
				  (void (*)(void))jp_ll_rw_block);
#endif /*__powerpc__*/

#else /* USE_SYMBOL_NAME */
		lkdtm_lookup_name("ll_rw_block",
				  (void (*)(void))jp_ll_rw_block);

#endif /* USE_SYMBOL_NAME */
		break;

	case MEM_SWAPOUT:

#ifdef USE_SYMBOL_NAME

#ifdef __powerpc__
		lkdtm_symbol_name(".shrink_inactive_list",
				  (void (*)(void))jp_shrink_page_list);
#else
		lkdtm_symbol_name("shrink_inactive_list",
				  (void (*)(void))jp_shrink_page_list);
#endif /*__powerpc__*/

#else /* USE_SYMBOL_NAME */
		lkdtm_lookup_name("shrink_inactive_list",
				  (void (*)(void))jp_shrink_page_list);

#endif /* USE_SYMBOL_NAME */
		break;

	case TIMERADD:

#ifdef USE_SYMBOL_NAME

#ifdef __powerpc__
		lkdtm_symbol_name(".hrtimer_start",
				  (void (*)(void))jp_hrtimer_start);
#else
		lkdtm_symbol_name("hrtimer_start",
				  (void (*)(void))jp_hrtimer_start);
#endif /*__powerpc__*/

#else /* USE_SYMBOL_NAME */
		lkdtm_lookup_name("hrtimer_start",
				  (void (*)(void))jp_hrtimer_start);

#endif /* USE_SYMBOL_NAME */
		break;

	case SCSI_DISPATCH_CMD:

#ifdef USE_SYMBOL_NAME

#ifdef __powerpc__
		lkdtm_symbol_name(".scsi_dispatch_cmd",
				  (void (*)(void))jp_scsi_dispatch_cmd);
#else
		lkdtm_symbol_name("scsi_dispatch_cmd",
				  (void (*)(void))jp_scsi_dispatch_cmd);
#endif /*__powerpc__*/

#else /* USE_SYMBOL_NAME */
		lkdtm_lookup_name("scsi_dispatch_cmd",
				  (void (*)(void))jp_scsi_dispatch_cmd);

#endif /* USE_SYMBOL_NAME */
		break;

	case IDE_CORE_CP:
#ifdef CONFIG_IDE

#ifdef USE_SYMBOL_NAME

#ifdef __powerpc__
		lkdtm_symbol_name(".scsi_dispatch_cmd",
				  (void (*)(void))jp_scsi_dispatch_cmd);
#else
		lkdtm_symbol_name("scsi_dispatch_cmd",
				  (void (*)(void))jp_scsi_dispatch_cmd);
#endif /*__powerpc__*/

#else /* USE_SYMBOL_NAME */
		lkdtm_lookup_name("scsi_dispatch_cmd",
				  (void (*)(void))jp_scsi_dispatch_cmd);

#endif /* USE_SYMBOL_NAME */
#endif /* CONFIG_IDE */
		break;

	default:
		printk(KERN_INFO "lkdtm : Invalid Crash Point\n");
		break;
	}

	if ((ret = register_jprobe(&lkdtm)) < 0) {
		printk(KERN_INFO "lkdtm : Couldn't register jprobe\n");
		return ret;
	}

	printk(KERN_INFO "lkdtm : Crash point %s of type %s registered\n",
	       cpoint_name, cpoint_type);
	return 0;
}

void lkdtm_module_exit(void)
{
	unregister_jprobe(&lkdtm);
	printk(KERN_INFO "lkdtm : Crash point unregistered\n");
}

module_init(lkdtm_module_init);
module_exit(lkdtm_module_exit);

MODULE_LICENSE("GPL");
