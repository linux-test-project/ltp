// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) Linux Test Project, 2006-2021
 * Copyright (c) 2000 Silicon Graphics, Inc.  All Rights Reserved.
 * Author: William Roske
 * Ported to LTP: Dave Fenner
 */

/*\
 * Basic test for fchown(). Call fchown() on a fd and expect it to pass.
 */

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "tst_test.h"
#include "compat_tst_16.h"

#define FILENAME "fchown01_testfile"
#define MODE 0700

static int fd;
static uid_t uid;
static gid_t gid;

static void run(void)
{
	TST_EXP_PASS(FCHOWN(fd, uid, gid),
		"fchown(%i, %i, %i)", fd, uid, gid);
}

static void setup(void)
{
	UID16_CHECK(uid = geteuid(), "fchown");
	GID16_CHECK(gid = getegid(), "fchown");
	fd = SAFE_OPEN(FILENAME, O_RDWR | O_CREAT, MODE);
}

static void cleanup(void)
{
	if (fd > 0)
		SAFE_CLOSE(fd);
}

static struct tst_test test = {
	.needs_tmpdir = 1,
	.setup = setup,
	.cleanup = cleanup,
	.test_all = run,
};
