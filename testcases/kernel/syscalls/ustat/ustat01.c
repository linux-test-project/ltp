// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) Wipro Technologies Ltd, 2003.  All Rights Reserved.
 *
 * Check that ustat() succeeds given correct parameters.
 */

#include "config.h"
#include "tst_test.h"

#if defined(HAVE_SYS_USTAT_H) || defined(HAVE_LINUX_TYPES_H)
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "lapi/syscalls.h"
#include "lapi/ustat.h"

static dev_t dev_num;

void run(void)
{
	struct ustat ubuf;

	TEST(tst_syscall(__NR_ustat, (unsigned int)dev_num, &ubuf));

	if (TST_RET == -1)
		tst_res(TFAIL | TTERRNO, "ustat(2) failed");
	else
		tst_res(TPASS, "ustat(2) passed");
}

static void setup(void)
{
	struct stat buf;

	/* Find a valid device number */
	SAFE_STAT("/", &buf);

	dev_num = buf.st_dev;
}

static struct tst_test test = {
	.test_all = run,
	.setup = setup,
};
#else
TST_TEST_TCONF("testing ustat requires <sys/ustat.h> or <linux/types.h>");
#endif
