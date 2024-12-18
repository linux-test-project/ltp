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

#define MB			(1UL<<20)
#define KB			(1UL<<10)
#define PATH_SYSVM		"/proc/sys/vm/"
#define PATH_MEMINFO		"/proc/meminfo"

/* KSM */

void create_same_memory(int size, int num, int unit);
void test_ksm_merge_across_nodes(unsigned long nr_pages);
void ksm_group_check(int run, int pg_shared, int pg_sharing, int pg_volatile,
                     int pg_unshared, int sleep_msecs, int pages_to_scan);

/* HUGETLB */

#define PATH_SHMMAX		"/proc/sys/kernel/shmmax"

void write_memcg(void);

#endif
