// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines  Corp., 2001
 *  07/2001 Ported by Wayne Boyer
 * Copyright (c) 2023 SUSE LLC Avinesh Kumar <avinesh.kumar@suse.com>
 */

/*\
 * [Description]
 *
 * Verify that, after a successful mmap() call, permission bits of the new
 * mapping in /proc/pid/maps file matches the prot and flags arguments in
 * mmap() call.
 */

#include <inttypes.h>
#include "tst_test.h"
#include <stdio.h>

#define MMAPSIZE 1024
static char *addr;

static struct tcase {
	int prot;
	int flags;
	char *exp_perms;
} tcases[] = {
	{PROT_NONE, MAP_ANONYMOUS | MAP_PRIVATE, "---p"},
	{PROT_NONE, MAP_ANONYMOUS | MAP_SHARED, "---s"},
	{PROT_READ, MAP_ANONYMOUS | MAP_PRIVATE, "r--p"},
	{PROT_READ, MAP_ANONYMOUS | MAP_SHARED, "r--s"},
	{PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, "-w-p"},
	{PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, "-w-s"},
	{PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, "rw-p"},
	{PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, "rw-s"},
	{PROT_READ | PROT_EXEC, MAP_ANONYMOUS | MAP_PRIVATE, "r-xp"},
	{PROT_READ | PROT_EXEC, MAP_ANONYMOUS | MAP_SHARED, "r-xs"},
	{PROT_WRITE | PROT_EXEC, MAP_ANONYMOUS | MAP_PRIVATE, "-wxp"},
	{PROT_WRITE | PROT_EXEC, MAP_ANONYMOUS | MAP_SHARED, "-wxs"},
	{PROT_READ | PROT_WRITE | PROT_EXEC, MAP_ANONYMOUS | MAP_PRIVATE, "rwxp"},
	{PROT_READ | PROT_WRITE | PROT_EXEC, MAP_ANONYMOUS | MAP_SHARED, "rwxs"}
};

static void run(unsigned int i)
{
	struct tcase *tc = &tcases[i];
	char addr_str[20];
	char perms[8];
	char fmt[1024];

	addr = SAFE_MMAP(NULL, MMAPSIZE, tc->prot, tc->flags, -1, 0);

	sprintf(addr_str, "%" PRIxPTR, (uintptr_t)addr);
	sprintf(fmt, "%s-%%*x %%s", addr_str);
	SAFE_FILE_LINES_SCANF("/proc/self/maps", fmt, perms);

	if (!strcmp(perms, tc->exp_perms)) {
		tst_res(TPASS, "mapping permissions in /proc matched: %s", perms);
	} else {
		tst_res(TFAIL, "mapping permissions in /proc mismatched, expected: %s, found: %s",
						tc->exp_perms, perms);
	}

	SAFE_MUNMAP(addr, MMAPSIZE);
}

static struct tst_test test = {
	.test = run,
	.tcnt = ARRAY_SIZE(tcases),
};
