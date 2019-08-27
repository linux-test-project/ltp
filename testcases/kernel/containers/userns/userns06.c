/*
 * Copyright (c) Huawei Technologies Co., Ltd., 2015
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version. This program is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
 * Public License for more details. You should have received a copy of the GNU
 * General Public License along with this program.
 */

/*
 * Verify that:
 * When a process with non-zero user IDs performs an execve(), the process's
 * capability sets are cleared.
 * When a process with zero user IDs performs an execve(), the process's
 * capability sets are set.
 *
 */

#define _GNU_SOURCE
#include <sys/wait.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include "libclone.h"
#include "test.h"
#include "config.h"
#include "userns_helper.h"

#define CHILD1UID 0
#define CHILD1GID 0
#define CHILD2UID 200
#define CHILD2GID 200

char *TCID = "user_namespace6";
int TST_TOTAL = 1;

static int cpid1, parentuid, parentgid;

/*
 * child_fn1() - Inside a new user namespace
 */
static int child_fn1(void)
{
	int exit_val = 0;
	char *const args[] = { "userns06_capcheck", "privileged", NULL };

	TST_SAFE_CHECKPOINT_WAIT(NULL, 0);

	if (execve(args[0], args, NULL) == -1) {
		printf("execvp unexpected error: (%d) %s\n",
			errno, strerror(errno));
		exit_val = 1;
	}

	return exit_val;
}

/*
 * child_fn2() - Inside a new user namespace
 */
static int child_fn2(void)
{
	int exit_val = 0;
	int uid, gid;
	char *const args[] = { "userns06_capcheck", "unprivileged", NULL };

	TST_SAFE_CHECKPOINT_WAIT(NULL, 1);

	uid = geteuid();
	gid = getegid();

	if (uid != CHILD2UID || gid != CHILD2GID) {
		printf("unexpected uid=%d gid=%d\n", uid, gid);
		exit_val = 1;
	}

	if (execve(args[0], args, NULL) == -1) {
		printf("execvp unexpected error: (%d) %s\n",
			errno, strerror(errno));
		exit_val = 1;
	}

	return exit_val;
}

static void cleanup(void)
{
	tst_rmdir();
}

static void setup(void)
{
	check_newuser();
	tst_tmpdir();
	TST_CHECKPOINT_INIT(NULL);
	TST_RESOURCE_COPY(cleanup, "userns06_capcheck", NULL);
}

int main(int argc, char *argv[])
{
	pid_t cpid2;
	char path[BUFSIZ];
	int lc;
	int fd;

	tst_parse_opts(argc, argv, NULL, NULL);
#ifndef HAVE_LIBCAP
	tst_brkm(TCONF, NULL, "System is missing libcap.");
#endif
	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		tst_count = 0;

		parentuid = geteuid();
		parentgid = getegid();

		cpid1 = ltp_clone_quick(CLONE_NEWUSER | SIGCHLD,
			(void *)child_fn1, NULL);
		if (cpid1 < 0)
			tst_brkm(TBROK | TERRNO, cleanup,
				"cpid1 clone failed");

		cpid2 = ltp_clone_quick(CLONE_NEWUSER | SIGCHLD,
			(void *)child_fn2, NULL);
		if (cpid2 < 0)
			tst_brkm(TBROK | TERRNO, cleanup,
				"cpid2 clone failed");

		if (access("/proc/self/setgroups", F_OK) == 0) {
			sprintf(path, "/proc/%d/setgroups", cpid1);
			fd = SAFE_OPEN(cleanup, path, O_WRONLY, 0644);
			SAFE_WRITE(cleanup, 1, fd, "deny", 4);
			SAFE_CLOSE(cleanup, fd);

			sprintf(path, "/proc/%d/setgroups", cpid2);
			fd = SAFE_OPEN(cleanup, path, O_WRONLY, 0644);
			SAFE_WRITE(cleanup, 1, fd, "deny", 4);
			SAFE_CLOSE(cleanup, fd);
		}

		updatemap(cpid1, UID_MAP, CHILD1UID, parentuid, cleanup);
		updatemap(cpid2, UID_MAP, CHILD2UID, parentuid, cleanup);

		updatemap(cpid1, GID_MAP, CHILD1GID, parentgid, cleanup);
		updatemap(cpid2, GID_MAP, CHILD2GID, parentgid, cleanup);

		TST_SAFE_CHECKPOINT_WAKE(cleanup, 0);
		TST_SAFE_CHECKPOINT_WAKE(cleanup, 1);

		tst_record_childstatus(cleanup, cpid1);
		tst_record_childstatus(cleanup, cpid2);
	}
	cleanup();
	tst_exit();
}
