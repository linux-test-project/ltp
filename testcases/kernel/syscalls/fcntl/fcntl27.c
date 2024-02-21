// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines  Corp., 2004
 * Copyright (C) Bull S.A.S 2005-2006
 * Author: Jacky Malcles
 * Copyright (C) 2024 SUSE LLC Andrea Manzini <andrea.manzini@suse.com>
 */

/*\
 * [Description]
 *
 * Basic test for fcntl(2) using F_SETLEASE & F_RDLCK argument.
 */

#include "lapi/fcntl.h"
#include "tst_test.h"

static struct test_case
{
	int oflags;
	mode_t mode;
} tcases[] = {
	{O_RDWR | O_CREAT, 0777},
	{O_WRONLY | O_CREAT, 0222},
};

static void verify_fcntl(unsigned int nr)
{
	struct test_case *tc = &tcases[nr];
	int fd = SAFE_OPEN("testfile", tc->oflags, tc->mode);

	TST_EXP_FAIL(fcntl(fd, F_SETLEASE, F_RDLCK), EAGAIN);
	SAFE_CLOSE(fd);
}

static struct tst_test test = {
	.test = verify_fcntl,
	.tcnt = ARRAY_SIZE(tcases),
	.needs_tmpdir = 1,
};
