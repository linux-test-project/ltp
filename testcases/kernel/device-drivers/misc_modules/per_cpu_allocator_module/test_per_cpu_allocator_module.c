/******************************************************************************/
/*                                                                            */
/* Copyright (c) Tejun Heo <tj@kernel.org>, 2009                              */
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
/******************************************************************************/
#include <linux/module.h>
#include <linux/device.h>
#include <linux/sysfs.h>
#include <linux/percpu.h>

static inline void pcpu_dump_chunk_slots(void)
{
}

struct alloc_cmd {
	size_t size;
	int marker;
};

static const struct alloc_cmd cmds[] = {
	{256, 0}, {256, 1}, {256, 0}, {256, 1},
	{256, 0}, {256, 1}, {256, 0}, {256, 1},
	{256, 0}, {256, 1}, {256, 0}, {256, 1},
	{256, 0}, {256, 1}, {256, 0}, {256, 1},
	{256, 0}, {256, 1}, {256, 0}, {256, 1},
	{256, 0}, {256, 1}, {256, 0}, {256, 1},
	{256, 0}, {256, 1}, {256, 0}, {256, 1},
	{256, 0}, {256, 1}, {256, 0}, {256, 1},	/* 8k */

	{1024, 2}, {1024, 2}, {1024, 2}, {1024, 2},
	{1024, 2}, {1024, 2}, {1024, 2}, {1024, 2},
	{1024, 2}, {1024, 2}, {1024, 2}, {1024, 2},
	{1024, 2}, {1024, 2}, {1024, 2}, {1024, 2},
	{1024, 2}, {1024, 2}, {1024, 2}, {1024, 2},
	{1024, 2}, {1024, 2}, {1024, 2}, {1024, 2},
	{1024, 2}, {1024, 2}, {1024, 2}, {1024, 2},
	{1024, 2}, {1024, 2}, {1024, 2}, {1024, 2},	/* 32k */

	{8192, 3}, {8192, 3}, {8192, 3}, {8192, 3},
	{8192, 3}, {8192, 3}, {8192, 3}, {8192, 3},
	{8192, 3}, {8192, 3}, {8192, 3}, {8192, 3},
	{8192, 3}, {8192, 3}, {8192, 3}, {8192, 3},
	{8192, 3}, {8192, 3}, {8192, 3}, {8192, 3},
	{8192, 3}, {8192, 3}, {8192, 3}, {8192, 3},
	{8192, 3}, {8192, 3}, {8192, 3}, {8192, 3},
	{8192, 3}, {8192, 3}, {8192, 3}, {8192, 3},	/* 128k */

	{0, 0},			/* free 0s */

	{128, 4}, {128, 4}, {128, 4}, {128, 4},
	{128, 4}, {128, 4}, {128, 4}, {128, 4},
	{128, 4}, {128, 4}, {128, 4}, {128, 4},
	{128, 4}, {128, 4}, {128, 4}, {128, 4},
	{128, 4}, {128, 4}, {128, 4}, {128, 4},
	{128, 4}, {128, 4}, {128, 4}, {128, 4},
	{128, 4}, {128, 4}, {128, 4}, {128, 4},
	{128, 4}, {128, 4}, {128, 4}, {128, 4},
	{128, 4}, {128, 4}, {128, 4}, {128, 4},	/* 4.5k */

	{0, 1},			/* free 1s */

	{1024, 5}, {1024, 5}, {1024, 5}, {1024, 5},	/* 4k */

	{0, 5},			/* free 5s */
	{0, 4},
	{0, 3},
	{0, 2},
};

#define NR_CMDS		ARRAY_SIZE(cmds)

static void *ptrs[NR_CMDS];

static unsigned long seed_val(unsigned int cmdno, unsigned int cpu)
{
	return 0xdeadbeefbeefdeadULL
	    + cmdno + (cmdno << 16) + (cpu << 8) + (cpu << 24);
}

static void fill_area(void *p, size_t size, unsigned int cmdno)
{
	unsigned int cpu;

	for_each_possible_cpu(cpu) {
		unsigned long v = seed_val(cmdno, cpu);
		unsigned long *up = per_cpu_ptr(p, cpu);
		size_t left = size;

		while (left >= sizeof(unsigned long)) {
			*up++ = v++;
			left -= sizeof(unsigned long);
		}
	}
}

static void verify_area(void *p, size_t size, unsigned int cmdno)
{
	unsigned int warns = 5;
	unsigned int cpu;

	for_each_possible_cpu(cpu) {
		unsigned long v = seed_val(cmdno, cpu);
		unsigned long *up = per_cpu_ptr(p, cpu);
		size_t left = size;

		while (left >= sizeof(unsigned long)) {
			if (*up != v && warns-- > 0) {
				printk
				    ("MISMATCH: cmdno=%u size=%zu cpu=%u off=%zu p=%p\n",
				     cmdno, size, cpu, size - left, p);
				printk("          [%p]=%lx should be %lx\n", up,
				       *up, v);
			}
			up++;
			v++;
			left -= sizeof(unsigned long);
		}
	}
}

static void free_cmd(unsigned int cmdno)
{
	if (!ptrs[cmdno])
		return;

	verify_area(ptrs[cmdno], cmds[cmdno].size, cmdno);
	free_percpu(ptrs[cmdno]);
	ptrs[cmdno] = NULL;
}

static void run_test(void)
{
	unsigned int i, j;

	for (i = 0; i < NR_CMDS; i++) {
		const struct alloc_cmd *cmd = &cmds[i];

		if (cmd->size) {
			printk("ALLOC: %zu bytes marker=%d\n",
			       cmd->size, cmd->marker);
			ptrs[i] = __alloc_percpu(cmd->size,
						 __alignof__(unsigned long
							     long));
			if (ptrs[i])
				fill_area(ptrs[i], cmd->size, i);
			else
				printk("failed to allocate %zu bytes\n",
				       cmd->size);
			continue;
		}

		printk("FREE: marker=%d\n", cmd->marker);
		pcpu_dump_chunk_slots();
		for (j = 0; j < i; j++)
			if (cmds[j].marker == cmd->marker)
				free_cmd(j);
		printk("FREE: done\n");
		pcpu_dump_chunk_slots();
	}
}

struct stupid_large_alignment_struct {
	int a;
	int b;
	int c;
} __aligned(2048);

struct stupid_large_struct {
	char a[1024];
};

static DEFINE_PER_CPU(struct stupid_large_alignment_struct, blah_blah);
DEFINE_PER_CPU(struct stupid_large_struct, blah_blah2);
DEFINE_PER_CPU(struct stupid_large_struct, blah_blah3);
DEFINE_PER_CPU(struct stupid_large_struct, blah_blah4);
DEFINE_PER_CPU(struct stupid_large_struct, blah_blah5);
DEFINE_PER_CPU(struct stupid_large_struct, blah_blah6);

static int __init test_init(void)
{
	unsigned int cpu;

	printk("XXX test_pcpu:");
	for_each_possible_cpu(cpu)
	    printk(" %p", &per_cpu(blah_blah, cpu));
	printk("\n");
	pcpu_dump_chunk_slots();

	run_test();
	return 0;
}

static void __exit test_exit(void)
{
	unsigned int i;

	printk("XXX cleaning up\n");
	pcpu_dump_chunk_slots();

	for (i = 0; i < NR_CMDS; i++)
		free_cmd(i);

	printk("XXX done\n");
	pcpu_dump_chunk_slots();
}

module_init(test_init);
module_exit(test_exit);
MODULE_LICENSE("GPL");
