// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2017 Richard Palethorpe <rpalethorpe@suse.com>
 * Copyright (c) 2017 Fujitsu Ltd. (Xiao Yang <yangx.jy@cn.fujitsu.com>)
 */
/*
 * Originated as a test for CVE-2017-5669 but as it turns out the CVE was bogus
 * to begin with and the test was changed into a regression test for commit:
 *
 * commit 8f89c007b6dec16a1793cb88de88fcc02117bbbc
 * Author: Davidlohr Bueso <dave@stgolabs.net>
 * Date:   Fri May 25 14:47:30 2018 -0700
 *
 *  ipc/shm: fix shmat() nil address after round-down when remapping
 *
 * Which makes sure that SHM_REMAP forbids NULL address consistently for
 * SHM_RND as well.
 *
 * The timeline went as:
 *
 * 95e91b831f87 (ipc/shm: Fix shmat mmap nil-page protection)
 * a73ab244f0da (Revert "ipc/shm: Fix shmat mmap nil-page protect...)
 * 8f89c007b6de (ipc/shm: fix shmat() nil address after round-dow...)
 *
 * The original commit disallowed SHM_RND maps to zero (and rounded) entirely
 * and that broke userland for cases like Xorg.
 *
 * See also https://github.com/linux-test-project/ltp/issues/319
 *
 * This test needs root permissions or else security_mmap_addr(), from
 * get_unmapped_area(), will cause permission errors when trying to mmap lower
 * addresses.
 */

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#include <stdio.h>
#include <errno.h>
#include <string.h>

#include "tst_test.h"
#include "tst_safe_sysv_ipc.h"

static int shm_id;
static void *shm_addr;

static void setup(void)
{
	shm_id = SAFE_SHMGET(IPC_PRIVATE, getpagesize(), 0777);
}

static void cleanup(void)
{
	if (shm_addr)
		SAFE_SHMDT(shm_addr);

	if (shm_id)
		SAFE_SHMCTL(shm_id, IPC_RMID, 0);
}

static void run(void)
{
	tst_res(TINFO, "Attempting to attach shared memory to null page");
	/*
	 * shmat() for 0 (or < PAGESIZE with RND flag) has to fail with REMAPs
	 * https://github.com/linux-test-project/ltp/issues/319
	 */
	shm_addr = shmat(shm_id, ((void *)1), SHM_RND | SHM_REMAP);
	if (shm_addr == (void *)-1) {
		shm_addr = NULL;
		if (errno == EINVAL) {
			tst_res(TPASS, "shmat returned EINVAL");
			return;
		}
		tst_brk(TBROK | TERRNO,
			"The bug was not triggered, but the shmat error is unexpected");
	}

	tst_res(TINFO, "Mapped shared memory to %p", shm_addr);

	if (!((size_t)shm_addr & (~0U << 16)))
		tst_res(TFAIL,
			"We have mapped a VM address within the first 64Kb");
	else
		tst_res(TPASS,
			"The kernel assigned a different VM address");

	tst_res(TINFO,
		"Touching shared memory to see if anything strange happens");
	((char *)shm_addr)[0] = 'P';

	SAFE_SHMDT(shm_addr);
	shm_addr = NULL;
}

static struct tst_test test = {
	.needs_root = 1,
	.setup = setup,
	.cleanup = cleanup,
	.test_all = run,
	.tags = (const struct tst_tag[]) {
		{"linux-git", "95e91b831f87"},
		{"linux-git", "a73ab244f0da"},
		{"linux-git", "8f89c007b6de"},
		{}
	}
};
