// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright © International Business Machines  Corp., 2007, 2008
 * Copyright (c) Linux Test Project, 2009-2024
 *
 */

/*\
 * [Description]
 *
 * The test process is affined to a CPU. It then calls getcpu and
 * checks that the CPU and node (if supported) match the expected
 * values.
 */

#define _GNU_SOURCE
#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include "tst_test.h"
#include "lapi/cpuset.h"
#include "lapi/sched.h"

static unsigned int max_cpuid(size_t size, cpu_set_t * set)
{
	unsigned int index, max = 0;
	for (index = 0; index < size * 8; index++)
		if (CPU_ISSET_S(index, size, set))
			max = index;
	return max;
}

/*
 * This will set the affinity to max cpu on which process can run
 * and return that cpu id to the calling process
 */
static unsigned int set_cpu_affinity(void)
{
	unsigned cpu_max;
	cpu_set_t *set;
	size_t size;
	int nrcpus = 1024;

realloc:
	set = CPU_ALLOC(nrcpus);
	if (!set)
		tst_brk(TBROK | TERRNO, "CPU_ALLOC()");

	size = CPU_ALLOC_SIZE(nrcpus);
	CPU_ZERO_S(size, set);
	if (sched_getaffinity(0, size, set) < 0) {
		CPU_FREE(set);
		if (errno == EINVAL && nrcpus < (1024 << 8)) {
			nrcpus = nrcpus << 2;
			goto realloc;
		}
		tst_brk(TBROK | TERRNO, "sched_getaffinity()");
	}
	cpu_max = max_cpuid(size, set);
	CPU_ZERO_S(size, set);
	CPU_SET_S(cpu_max, size, set);
	if (sched_setaffinity(0, size, set) < 0) {
		CPU_FREE(set);
		tst_brk(TBROK | TERRNO, "sched_setaffinity()");
	}
	CPU_FREE(set);
	return cpu_max;
}

static unsigned int get_nodeid(unsigned int cpu_id)
{
	DIR *directory_parent, *directory_node;
	struct dirent *de, *dn;
	char directory_path[PATH_MAX];
	char *invalid_number;
	unsigned int cpu;
	int node_id = 0;

	directory_parent = opendir("/sys/devices/system/node");
	if (!directory_parent) {
		tst_res(TINFO,
			"/sys not mounted or not a numa system. "
			"Assuming one node");
		tst_res(TINFO, "Error opening: /sys/devices/system/node :%s",
			strerror(errno));
		/* Assume CPU belongs to the only node, node zero. */
		return 0;
	} else {
		while ((de = readdir(directory_parent)) != NULL) {
			if (strncmp(de->d_name, "node", 4))
				continue;
			sprintf(directory_path, "/sys/devices/system/node/%s",
				de->d_name);
			directory_node = opendir(directory_path);
			while ((dn = readdir(directory_node)) != NULL) {
				if (strncmp(dn->d_name, "cpu", 3))
					continue;
				cpu = strtoul(dn->d_name + 3, &invalid_number, 0);
				if (strcmp(invalid_number, "\0"))
					continue;
				if (cpu == cpu_id) {
					node_id =
					    strtoul(de->d_name + 4, NULL, 0);
					break;
				}
			}
			closedir(directory_node);
		}
		closedir(directory_parent);
	}
	return node_id;
}

static void run(void)
{
	unsigned int cpu_id, node_id = 0;
	unsigned int cpu_set;
	unsigned int node_set;

	cpu_set = set_cpu_affinity();
	node_set = get_nodeid(cpu_set);

	TEST(getcpu(&cpu_id, &node_id));
	if (TST_RET == 0) {
		if (cpu_id != cpu_set)
			tst_res(TFAIL, "getcpu() returned wrong value"
				" expected cpuid:%d, returned value cpuid: %d",
				cpu_set, cpu_id);
		else if (node_id != node_set)
			tst_res(TFAIL, "getcpu() returned wrong value"
				" expected  node id:%d returned  node id:%d",
				node_set, node_id);
		else
			tst_res(TPASS, "getcpu() returned proper"
				" cpuid:%d, node id:%d", cpu_id,
				node_id);
	} else {
		tst_res(TFAIL, "getcpu() Failed, errno=%d:%s",
			TST_ERR, strerror(TST_ERR));
	}
}

static struct tst_test test = {
	.test_all = run,
};
