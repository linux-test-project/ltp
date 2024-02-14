// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) Wipro Technologies Ltd, 2002.  All Rights Reserved.
 *
 * Test whether ustat(2) system call returns appropriate error number for
 * invalid dev_t parameter and for bad address paramater.
 */

#include "config.h"
#include "tst_test.h"

#if defined(HAVE_SYS_USTAT_H) || defined(HAVE_LINUX_TYPES_H)
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "lapi/syscalls.h"
#include "lapi/ustat.h"

static dev_t invalid_dev = -1;
static dev_t root_dev;
struct ustat ubuf;

static struct test_case_t {
	char *err_desc;
	int exp_errno;
	char *exp_errval;
	dev_t *dev;
	struct ustat *buf;
} tc[] = {
	{"Invalid parameter", EINVAL, "EINVAL", &invalid_dev, &ubuf},
#ifndef UCLINUX
	{"Bad address", EFAULT, "EFAULT", &root_dev, (void*)-1}
#endif
};

int TST_TOTAL = ARRAY_SIZE(tc);

void run(unsigned int test)
{
	TEST(tst_syscall(__NR_ustat, (unsigned int)*tc[test].dev, tc[test].buf));

	if ((TST_RET == -1) && (TST_ERR == tc[test].exp_errno))
		tst_res(TPASS | TTERRNO, "ustat(2) expected failure");
	else
		tst_res(TFAIL | TTERRNO,
			"ustat(2) failed to produce expected error; %d, errno"
			": %s", tc[test].exp_errno, tc[test].exp_errval);
}

static void setup(void)
{
	struct stat buf;

	/* Find a valid device number */
	SAFE_STAT("/", &buf);

	root_dev = buf.st_dev;
}

static struct tst_test test = {
	.test = run,
	.setup = setup,
	.tcnt = ARRAY_SIZE(tc),
	.tags = (const struct tst_tag[]) {
		{"known-fail", "ustat() is known to fail with EINVAL on Btrfs, see "
			"https://lore.kernel.org/linux-btrfs/e7e867b8-b57a-7eb2-2432-1627bd3a88fb@toxicpanda.com/"
		},
		{}
	}
};
#else
TST_TEST_TCONF("testing ustat requires <sys/ustat.h> or <linux/types.h>");
#endif
