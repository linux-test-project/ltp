// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2000 Silicon Graphics, Inc.  All Rights Reserved.
 * AUTHOR : William Roske, CO-PILOT	: Dave Fenner
 * Copyright (c) 2022 SUSE LLC Avinesh Kumar <avinesh.kumar@suse.com>
 */

/*\
 * Verify that statfs() syscall executes successfully on all
 * available filesystems.
 */

#include "tst_test.h"

#define MNT_POINT "mntpoint"
#define TEMP_FILE MNT_POINT"/testfile"
#define TEXT "dummy text"

static void setup(void)
{
	int fd = SAFE_OPEN(TEMP_FILE, O_RDWR | O_CREAT, 0700);

	SAFE_WRITE(SAFE_WRITE_ALL, fd, TEXT, strlen(TEXT));
	SAFE_CLOSE(fd);
}

static void run(void)
{
	struct statfs buf;

	TST_EXP_PASS(statfs(TEMP_FILE, &buf));
}

static struct tst_test test = {
	.setup = setup,
	.test_all = run,
	.needs_root = 1,
	.mount_device = 1,
	.mntpoint = MNT_POINT,
	.all_filesystems = 1
};
