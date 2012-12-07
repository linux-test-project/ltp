/******************************************************************************/
/*                                                                            */
/* Copyright (c) Mathieu Desnoyers <mathieu.desnoyers@polymtl.ca>, 2009       */
/*                                                                            */
/* This program is free software;  you can redistribute it and/or modify      */
/* it under the terms of the GNU General Public License as published by       */
/* the Free Software Foundation; either version 2 of the License, or          */
/* (at your option) any later version.                                        */
/*                                                                            */
/* This program is distributed in the hope that it will be useful,            */
/* but WITHOUT ANY WARRANTY;  without even the implied warranty of            */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See                  */
/* the GNU General Public License for more details.                           */
/*                                                                            */
/* You should have received a copy of the GNU General Public License          */
/* along with this program;  if not, write to the Free Software               */
/* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA    */
/*                                                                            */
/* usage :
        make
        insmod test-cmpxchg-nolock.ko
        insmod: error inserting 'test-cmpxchg-nolock.ko':
                -1 Resource temporarily unavailable
        dmesg (see dmesg output)                                              */
/******************************************************************************/

/* test-cmpxchg-nolock.c
*
* Compare local cmpxchg with irq disable / enable.
*/

#include <linux/jiffies.h>
#include <linux/compiler.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/math64.h>
#include <asm/timex.h>
#include <asm/system.h>

#define NR_LOOPS 20000

int test_val;

static void do_testbaseline(void)
{
	unsigned long flags;
	unsigned int i;
	cycles_t time1, time2, time;
	u32 rem;

	local_irq_save(flags);
	preempt_disable();
	time1 = get_cycles();
	for (i = 0; i < NR_LOOPS; i++) {
		asm volatile ("");
	}
	time2 = get_cycles();
	local_irq_restore(flags);
	preempt_enable();
	time = time2 - time1;

	printk(KERN_ALERT "test results: time for baseline\n");
	printk(KERN_ALERT "number of loops: %d\n", NR_LOOPS);
	printk(KERN_ALERT "total time: %llu\n", time);
	time = div_u64_rem(time, NR_LOOPS, &rem);
	printk(KERN_ALERT "-> baseline takes %llu cycles\n", time);
	printk(KERN_ALERT "test end\n");
}

static void do_test_sync_cmpxchg(void)
{
	int ret;
	unsigned long flags;
	unsigned int i;
	cycles_t time1, time2, time;
	u32 rem;

	local_irq_save(flags);
	preempt_disable();
	time1 = get_cycles();
	for (i = 0; i < NR_LOOPS; i++) {
#ifdef CONFIG_X86_32
		ret = sync_cmpxchg(&test_val, 0, 0);
#else
		ret = cmpxchg(&test_val, 0, 0);
#endif
	}
	time2 = get_cycles();
	local_irq_restore(flags);
	preempt_enable();
	time = time2 - time1;

	printk(KERN_ALERT "test results: time for locked cmpxchg\n");
	printk(KERN_ALERT "number of loops: %d\n", NR_LOOPS);
	printk(KERN_ALERT "total time: %llu\n", time);
	time = div_u64_rem(time, NR_LOOPS, &rem);
	printk(KERN_ALERT "-> locked cmpxchg takes %llu cycles\n", time);
	printk(KERN_ALERT "test end\n");
}

static void do_test_cmpxchg(void)
{
	int ret;
	unsigned long flags;
	unsigned int i;
	cycles_t time1, time2, time;
	u32 rem;

	local_irq_save(flags);
	preempt_disable();
	time1 = get_cycles();
	for (i = 0; i < NR_LOOPS; i++) {
		ret = cmpxchg_local(&test_val, 0, 0);
	}
	time2 = get_cycles();
	local_irq_restore(flags);
	preempt_enable();
	time = time2 - time1;

	printk(KERN_ALERT "test results: time for non locked cmpxchg\n");
	printk(KERN_ALERT "number of loops: %d\n", NR_LOOPS);
	printk(KERN_ALERT "total time: %llu\n", time);
	time = div_u64_rem(time, NR_LOOPS, &rem);
	printk(KERN_ALERT "-> non locked cmpxchg takes %llu cycles\n", time);
	printk(KERN_ALERT "test end\n");
}

static void do_test_sync_inc(void)
{
	int ret;
	unsigned long flags;
	unsigned int i;
	cycles_t time1, time2, time;
	u32 rem;
	atomic_t val;

	local_irq_save(flags);
	preempt_disable();
	time1 = get_cycles();
	for (i = 0; i < NR_LOOPS; i++) {
		ret = atomic_add_return(10, &val);
	}
	time2 = get_cycles();
	local_irq_restore(flags);
	preempt_enable();
	time = time2 - time1;

	printk(KERN_ALERT "test results: time for locked add return\n");
	printk(KERN_ALERT "number of loops: %d\n", NR_LOOPS);
	printk(KERN_ALERT "total time: %llu\n", time);
	time = div_u64_rem(time, NR_LOOPS, &rem);
	printk(KERN_ALERT "-> locked add return takes %llu cycles\n", time);
	printk(KERN_ALERT "test end\n");
}

static void do_test_inc(void)
{
	int ret;
	unsigned long flags;
	unsigned int i;
	cycles_t time1, time2, time;
	u32 rem;
	local_t loc_val;

	local_irq_save(flags);
	preempt_disable();
	time1 = get_cycles();
	for (i = 0; i < NR_LOOPS; i++) {
		ret = local_add_return(10, &loc_val);
	}
	time2 = get_cycles();
	local_irq_restore(flags);
	preempt_enable();
	time = time2 - time1;

	printk(KERN_ALERT "test results: time for non locked add return\n");
	printk(KERN_ALERT "number of loops: %d\n", NR_LOOPS);
	printk(KERN_ALERT "total time: %llu\n", time);
	time = div_u64_rem(time, NR_LOOPS, &rem);
	printk(KERN_ALERT "-> non locked add return takes %llu cycles\n", time);
	printk(KERN_ALERT "test end\n");
}

/*
 * This test will have a higher standard deviation due to incoming interrupts.
 */
static void do_test_enable_int(void)
{
	unsigned long flags;
	unsigned int i;
	cycles_t time1, time2, time;
	u32 rem;

	local_irq_save(flags);
	preempt_disable();
	time1 = get_cycles();
	for (i = 0; i < NR_LOOPS; i++) {
		local_irq_restore(flags);
	}
	time2 = get_cycles();
	local_irq_restore(flags);
	preempt_enable();
	time = time2 - time1;

	printk(KERN_ALERT "test results: time for enabling interrupts (STI)\n");
	printk(KERN_ALERT "number of loops: %d\n", NR_LOOPS);
	printk(KERN_ALERT "total time: %llu\n", time);
	time = div_u64_rem(time, NR_LOOPS, &rem);
	printk(KERN_ALERT "-> enabling interrupts (STI) takes %llu cycles\n",
	       time);
	printk(KERN_ALERT "test end\n");
}

static void do_test_disable_int(void)
{
	unsigned long flags, flags2;
	unsigned int i;
	cycles_t time1, time2, time;
	u32 rem;

	local_irq_save(flags);
	preempt_disable();
	time1 = get_cycles();
	for (i = 0; i < NR_LOOPS; i++) {
		local_irq_save(flags2);
	}
	time2 = get_cycles();
	local_irq_restore(flags);
	preempt_enable();
	time = time2 - time1;

	printk(KERN_ALERT
	       "test results: time for disabling interrupts (CLI)\n");
	printk(KERN_ALERT "number of loops: %d\n", NR_LOOPS);
	printk(KERN_ALERT "total time: %llu\n", time);
	time = div_u64_rem(time, NR_LOOPS, &rem);
	printk(KERN_ALERT "-> disabling interrupts (CLI) takes %llu cycles\n",
	       time);
	printk(KERN_ALERT "test end\n");
}

static void do_test_int(void)
{
	unsigned long flags;
	unsigned int i;
	cycles_t time1, time2, time;
	u32 rem;

	local_irq_save(flags);
	preempt_disable();
	time1 = get_cycles();
	for (i = 0; i < NR_LOOPS; i++) {
		local_irq_restore(flags);
		local_irq_save(flags);
	}
	time2 = get_cycles();
	local_irq_restore(flags);
	preempt_enable();
	time = time2 - time1;

	printk(KERN_ALERT
	       "test results: time for disabling/enabling interrupts (STI/CLI)\n");
	printk(KERN_ALERT "number of loops: %d\n", NR_LOOPS);
	printk(KERN_ALERT "total time: %llu\n", time);
	time = div_u64_rem(time, NR_LOOPS, &rem);
	printk(KERN_ALERT
	       "-> enabling/disabling interrupts (STI/CLI) takes %llu cycles\n",
	       time);
	printk(KERN_ALERT "test end\n");
}

static int ltt_test_init(void)
{
	printk(KERN_ALERT "test init\n");

	do_testbaseline();
	do_test_sync_cmpxchg();
	do_test_cmpxchg();
	do_test_sync_inc();
	do_test_inc();
	do_test_enable_int();
	do_test_disable_int();
	do_test_int();
	return -EAGAIN;		/* Fail will directly unload the module */
}

static void ltt_test_exit(void)
{
	printk(KERN_ALERT "test exit\n");
}

module_init(ltt_test_init)
    module_exit(ltt_test_exit)

    MODULE_LICENSE("GPL");
MODULE_AUTHOR("Mathieu Desnoyers");
MODULE_DESCRIPTION("Cmpxchg vs int Test");
