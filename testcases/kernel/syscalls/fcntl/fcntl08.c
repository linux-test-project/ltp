// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2000 Silicon Graphics, Inc.  All Rights Reserved.
 * Authors: William Roske, Dave Fenner
 * Copyright (c) 2014 Fujitsu Ltd.
 * Copyright (c) Linux Test Project, 2005-2015
 * Copyright (C) 2024 SUSE LLC Andrea Manzini <andrea.manzini@suse.com>
 */

/*\
 * Basic test for fcntl(2) using F_SETFL with flags O_NDELAY | O_APPEND | O_NONBLOCK.
 */

#include "lapi/fcntl.h"
#include "tst_test.h"

static int fd;

static void setup(void)
{
	fd = SAFE_OPEN("testfile", O_RDWR | O_CREAT, 0700);
}

static void cleanup(void)
{
	if (fd > 0)
		SAFE_CLOSE(fd);
}

static void run(void)
{
	TST_EXP_PASS(fcntl(fd, F_SETFL, O_NDELAY | O_APPEND | O_NONBLOCK));
}

static struct tst_test test = {
	.test_all = run,
	.setup = setup,
	.cleanup = cleanup,
	.needs_tmpdir = 1,
};
