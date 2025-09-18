// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines  Corp., 2001
 *  07/2001 Ported by Wayne Boyer
 * Copyright (c) 2023 SUSE LLC Avinesh Kumar <avinesh.kumar@suse.com>
 */

/*\
 * Verify that, after a successful mmap() call, permission bits of the new
 * mapping in /proc/pid/maps file matches the prot and flags arguments in
 * mmap() call.
 */

#include <inttypes.h>
#include "tst_test.h"
#include <stdio.h>

static char *addr1;
static char *addr2;

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
	char perms[8];
	char fmt[1024];
	unsigned int pagesize;
	int flag;

	pagesize = SAFE_SYSCONF(_SC_PAGESIZE);

	/* To avoid new mapping getting merged with existing mappings, we first
	 * create a 2-page mapping with the different permissions, and then remap
	 * the 2nd page with the perms being tested.
	 */
	flag = (tc->flags & MAP_PRIVATE) ? MAP_SHARED : MAP_PRIVATE;
	addr1 = SAFE_MMAP(NULL, pagesize * 2, PROT_NONE, MAP_ANONYMOUS | flag, -1, 0);

	addr2 = SAFE_MMAP(addr1 + pagesize, pagesize, tc->prot, tc->flags | MAP_FIXED, -1, 0);

	/* A /proc/self/maps address is at least 8 hex (left zero padded) */
	sprintf(fmt, "%08" PRIxPTR "-%%*x %%s", (uintptr_t)addr2);
	SAFE_FILE_LINES_SCANF("/proc/self/maps", fmt, perms);

	if (!strcmp(perms, tc->exp_perms)) {
		tst_res(TPASS, "mapping permissions in /proc matched: %s", perms);
	} else {
		tst_res(TFAIL, "mapping permissions in /proc mismatched, expected: %s, found: %s",
						tc->exp_perms, perms);
	}

	SAFE_MUNMAP(addr1, pagesize * 2);
}

static struct tst_test test = {
	.test = run,
	.tcnt = ARRAY_SIZE(tcases),
};
