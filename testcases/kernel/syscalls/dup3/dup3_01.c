// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) Ulrich Drepper <drepper@redhat.com>
 * Copyright (c) International Business Machines  Corp., 2009
 * Created - Jan 13 2009 - Ulrich Drepper <drepper@redhat.com>
 * Ported to LTP - Jan 13 2009 - Subrata <subrata@linux.vnet.ibm.com>
 */

/*\
 * Testcase to check whether dup3() supports O_CLOEXEC flag.
 */

#define _GNU_SOURCE

#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include "tst_test.h"
#include "tst_safe_macros.h"

static int fd = -1;

static struct tcase {
	int coe_flag;
	char *desc;
} tcases[] = {
	{0, "dup3(1, 4, 0)"},
	{O_CLOEXEC, "dup3(1, 4, O_CLOEXEC)"},
};

static void cleanup(void)
{
	if (fd > -1)
		close(fd);
}

static void run(unsigned int i)
{
	int ret;
	struct tcase *tc = tcases + i;
	TST_EXP_FD_SILENT(dup3(1, 4, tc->coe_flag), "dup3(1, 4, %d)", tc->coe_flag);

	fd = TST_RET;
	ret = SAFE_FCNTL(fd, F_GETFD);
	if (tc->coe_flag) {
		if (ret & FD_CLOEXEC)
			tst_res(TPASS, "%s set close-on-exec flag", tc->desc);
		else
			tst_res(TFAIL, "%s set close-on-exec flag", tc->desc);
	} else {
		if (ret & FD_CLOEXEC)
			tst_res(TFAIL, "%s set close-on-exec flag", tc->desc);
		else
			tst_res(TPASS, "%s set close-on-exec flag", tc->desc);
	}
};

static struct tst_test test = {
	.tcnt = ARRAY_SIZE(tcases),
	.test = run,
	.cleanup = cleanup,
};
