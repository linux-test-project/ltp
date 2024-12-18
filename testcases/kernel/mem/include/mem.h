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

/* HUGETLB */

#define PATH_SHMMAX		"/proc/sys/kernel/shmmax"

void write_memcg(void);

#endif
