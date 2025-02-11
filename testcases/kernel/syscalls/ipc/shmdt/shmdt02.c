// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines Corp., 2001
 */

/*\
 * Tests basic error handing of the shmdt syscall.
 *
 * -EINVAL there is no shared memory segment attached at shmaddr.
 * -EINVAL shmaddr is not aligned on a page boundary.
 */

#include <sys/types.h>
#include <sys/shm.h>
#include "tst_test.h"
#include "libnewipc.h"

static void *non_attched_addr;
static void *unaligned_addr;

struct tcase {
	void **addr;
	char *des;
} tcases[] = {
	{&non_attched_addr, "shmdt(non_attched_addr)"},
	{&unaligned_addr, "shmdt(unaligned_addr)"}
};

static void verify_shmdt(unsigned int n)
{
	struct tcase *tc = &tcases[n];

	TST_EXP_FAIL(shmdt(*tc->addr), EINVAL, "%s", tc->des);
}

static void setup(void)
{
	non_attched_addr = PROBE_FREE_ADDR();
	unaligned_addr = non_attched_addr + SHMLBA - 1;
}

static struct tst_test test = {
	.setup = setup,
	.test = verify_shmdt,
	.tcnt = ARRAY_SIZE(tcases),
};
