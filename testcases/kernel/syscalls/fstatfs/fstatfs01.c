// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2000 Silicon Graphics, Inc.  All Rights Reserved.
 * Copyright (c) 2022 SUSE LLC Avinesh Kumar <avinesh.kumar@suse.com>
 */

/*\
 * [Description]
 *
 * Verify that fstatfs() syscall executes successfully for all
 * available filesystems.
 */


#include <stdio.h>
#include "tst_test.h"

#define MNT_POINT "mntpoint"
#define TEMP_FILE MNT_POINT"/test_file"

static int file_fd;
static int pipe_fd;

static struct tcase {
	int *fd;
	const char *msg;
} tcases[] = {
	{&file_fd, "fstatfs() on a file"},
	{&pipe_fd, "fstatfs() on a pipe"},
};

static void run(unsigned int i)
{
	struct tcase *tc = &tcases[i];
	struct statfs buf;

	TST_EXP_PASS(fstatfs(*tc->fd, &buf), "%s", tc->msg);
}

static void setup(void)
{
	int pipe[2];

	file_fd = SAFE_OPEN(TEMP_FILE, O_RDWR | O_CREAT, 0700);
	SAFE_PIPE(pipe);
	pipe_fd = pipe[0];
	SAFE_CLOSE(pipe[1]);
}

static void cleanup(void)
{
	if (file_fd > 0)
		SAFE_CLOSE(file_fd);
	if (pipe_fd > 0)
		SAFE_CLOSE(pipe_fd);
}

static struct tst_test test = {
	.setup = setup,
	.cleanup = cleanup,
	.tcnt = ARRAY_SIZE(tcases),
	.test = run,
	.mount_device = 1,
	.mntpoint = MNT_POINT,
	.all_filesystems = 1,
	.needs_root = 1
};
