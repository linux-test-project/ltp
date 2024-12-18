// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) Linux Test Project, 2011-2021
 */
#ifndef _MEM_H
#define _MEM_H
#include "config.h"
#include "tst_test.h"
#include "ksm_helper.h"
#include "tst_memutils.h"

#if defined(__powerpc__) || defined(__powerpc64__)
#define MAXNODES		256
#else
#define MAXNODES		512
#endif
#define MB			(1UL<<20)
#define KB			(1UL<<10)
#define PATH_SYS_SYSTEM		"/sys/devices/system"
#define PATH_SYSVM		"/proc/sys/vm/"
#define PATH_MEMINFO		"/proc/meminfo"
#define BITS_PER_LONG           (8 * sizeof(long))

static inline void set_node(unsigned long *array, unsigned int node)
{
	array[node / BITS_PER_LONG] |= 1UL << (node % BITS_PER_LONG);
}

static inline void clean_node(unsigned long *array)
{
	unsigned int i;

	for (i = 0; i < MAXNODES / BITS_PER_LONG; i++)
		array[i] &= 0UL;
}

/* OOM */

#define LENGTH			(3UL<<30)
#define TESTMEM			(1UL<<30)
#define NORMAL			1
#define MLOCK			2
#define KSM			3

void oom(int testcase, int lite, int retcode, int allow_sigkill);
void testoom(int mempolicy, int lite, int retcode, int allow_sigkill);

/* KSM */

void create_same_memory(int size, int num, int unit);
void test_ksm_merge_across_nodes(unsigned long nr_pages);
void ksm_group_check(int run, int pg_shared, int pg_sharing, int pg_volatile,
                     int pg_unshared, int sleep_msecs, int pages_to_scan);

/* THP */

#define PATH_THP		"/sys/kernel/mm/transparent_hugepage/"
#define PATH_KHPD		PATH_THP "khugepaged/"

/* HUGETLB */

#define PATH_HUGEPAGES		"/sys/kernel/mm/hugepages/"
#define PATH_SHMMAX		"/proc/sys/kernel/shmmax"

void check_hugepage(void);
void write_memcg(void);

/* cpuset/memcg - include/tst_cgroup.h */
void write_cpusets(const struct tst_cg_group *cg, long nd);

#endif
