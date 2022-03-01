// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2022 FUJITSU LIMITED. All rights reserved.
 * Author: Yang Xu <xuyang2018.jy@fujitsu.com>
 */

/*\
 * [Description]
 *
 * Basic pidfd_getfd() test:
 *
 * - the close-on-exec flag is set on the file descriptor returned by
 *   pidfd_getfd
 * - use kcmp to check whether a file descriptor idx1 in the process pid1
 *   refers to the same open file description as file descriptor idx2 in
 *   the process pid2
 */

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include "tst_test.h"
#include "lapi/kcmp.h"
#include "tst_safe_macros.h"

#define TESTFILE "testfile"

static int fds[2] = {-1, -1};
static int pidfd = -1;

static void do_child(void)
{
	int fd;

	SAFE_CLOSE(fds[0]);
	fd = SAFE_CREAT(TESTFILE, 0644);
	SAFE_WRITE(1, fds[1], &fd, sizeof(fd));
	TST_CHECKPOINT_WAIT(0);
	SAFE_CLOSE(fd);
	SAFE_CLOSE(fds[1]);
	exit(0);
}

static void run(void)
{
	int flag, pid, targetfd, remotefd;

	SAFE_PIPE(fds);
	pid = SAFE_FORK();
	if (!pid)
		do_child();

	SAFE_CLOSE(fds[1]);
	TST_PROCESS_STATE_WAIT(pid, 'S', 0);

	pidfd = SAFE_PIDFD_OPEN(pid, 0);
	SAFE_READ(1, fds[0], &targetfd, sizeof(targetfd));
	TST_EXP_FD_SILENT(pidfd_getfd(pidfd, targetfd, 0),
		"pidfd_getfd(%d, %d , 0)", pidfd, targetfd);

	remotefd = TST_RET;
	flag = SAFE_FCNTL(remotefd, F_GETFD);
	if (!(flag & FD_CLOEXEC))
		tst_res(TFAIL, "pidfd_getfd() didn't set close-on-exec flag");

	TST_EXP_VAL_SILENT(kcmp(getpid(), pid, KCMP_FILE, remotefd, targetfd), 0);

	tst_res(TPASS, "pidfd_getfd(%d, %d, 0) passed", pidfd, targetfd);

	TST_CHECKPOINT_WAKE(0);
	SAFE_CLOSE(remotefd);
	SAFE_CLOSE(pidfd);
	SAFE_CLOSE(fds[0]);
	tst_reap_children();
}

static void setup(void)
{
	pidfd_open_supported();
	pidfd_getfd_supported();
}

static void cleanup(void)
{
	if (fds[0] > -1)
		SAFE_CLOSE(fds[0]);
	if (fds[1] > -1)
		SAFE_CLOSE(fds[1]);
	if (pidfd > -1)
		SAFE_CLOSE(pidfd);
}

static struct tst_test test = {
	.needs_root = 1,
	.needs_checkpoints = 1,
	.forks_child = 1,
	.setup = setup,
	.cleanup = cleanup,
	.test_all = run,
};
