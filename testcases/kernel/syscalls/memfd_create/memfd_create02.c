// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2017  Red Hat, Inc.
 */

 /*
  *  Based on Linux/tools/testing/selftests/memfd/memfd_test.c
  *  by David Herrmann <dh.herrmann@gmail.com>
  *
  *  24/02/2017   Port to LTP    <jracek@redhat.com>
  */

#define _GNU_SOURCE

#include <errno.h>
#include "memfd_create_common.h"
#include "tst_test.h"

static char buf[2048];
static char term_buf[2048];

static int available_flags;

static const struct tcase {
	char *descr;
	char *memfd_name;
	int flags;
	int memfd_create_exp_err;
} tcases[] = {
	/*
	 * Test memfd_create() syscall
	 * Verify syscall-argument validation, including name checks,
	 * flag validation and more.
	 */
	{"invalid name fail 1",   NULL,     0,                      EFAULT },
	{"invalid name fail 2",   buf,      0,                      EINVAL },
	{"invalid name fail 3",   term_buf, 0,                      EINVAL },

	{"invalid flags fail 1", "test",  -500,                     EINVAL },
	{"invalid flags fail 2", "test",  0x0100,                   EINVAL },
	{"invalid flags fail 3", "test",  ~MFD_CLOEXEC,             EINVAL },
	{"invalid flags fail 4", "test",  ~MFD_ALLOW_SEALING,       EINVAL },
	{"invalid flags fail 5", "test",  ~0,                       EINVAL },
	{"invalid flags fail 6", "test",  0x80000000U,              EINVAL },

	{"valid flags 1 pass", "test",  MFD_CLOEXEC,                     0 },
	{"valid flags 2 pass", "test",  MFD_ALLOW_SEALING,               0 },
	{"valid flags 3 pass", "test",  MFD_CLOEXEC | MFD_ALLOW_SEALING, 0 },
	{"valid flags 4 pass", "test",  0,                               0 },
	{"valid flags 5 pass", "",      0,                               0 },
};

static void setup(void)
{

	available_flags = GET_MFD_ALL_AVAILABLE_FLAGS();

	memset(buf, 0xff, sizeof(buf));

	memset(term_buf, 0xff, sizeof(term_buf));
	term_buf[sizeof(term_buf) - 1] = 0;
}

static void verify_memfd_create_errno(unsigned int n)
{
	const struct tcase *tc;
	int needed_flags;

	tc = &tcases[n];
	needed_flags = tc->flags & FLAGS_ALL_MASK;

	if ((available_flags & needed_flags) != needed_flags) {
		tst_res(TCONF, "test '%s' skipped, flag not implemented",
					tc->descr);
		return;
	}

	TEST(sys_memfd_create(tc->memfd_name, tc->flags));
	if (TST_ERR != tc->memfd_create_exp_err)
		tst_brk(TFAIL, "test '%s'", tc->descr);
	else
		tst_res(TPASS, "test '%s'", tc->descr);

	if (TST_RET > 0)
		SAFE_CLOSE(TST_RET);
}

static struct tst_test test = {
	.test = verify_memfd_create_errno,
	.tcnt = ARRAY_SIZE(tcases),
	.setup = setup,
};
