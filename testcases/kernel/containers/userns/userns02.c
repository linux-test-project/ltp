/*
 * Copyright (c) Huawei Technologies Co., Ltd., 2015
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See
 * the GNU General Public License for more details.
 */

/*
 * Verify that:
 *  The user ID and group ID, which are inside a container, can be modified
 * by its parent process.
 */

#define _GNU_SOURCE
#include <sys/wait.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include "userns_helper.h"
#include "test.h"

char *TCID = "user_namespace2";
int TST_TOTAL = 1;

static void cleanup(void)
{
	tst_rmdir();
}

/*
 * child_fn1() - Inside a new user namespace
 */
static int child_fn1(void)
{
	int exit_val;
	int uid, gid;

	TST_SAFE_CHECKPOINT_WAIT(NULL, 0);
	uid = geteuid();
	gid = getegid();

	if (uid == 100 && gid == 100) {
		printf("Got expected uid and gid.\n");
		exit_val = 0;
	} else {
		printf("Got unexpected result of uid=%d gid=%d\n", uid, gid);
		exit_val = 1;
	}

	return exit_val;
}

static void setup(void)
{
	check_newuser();
	tst_tmpdir();
	TST_CHECKPOINT_INIT(NULL);
}

int main(int argc, char *argv[])
{
	int lc;
	int childpid;
	int parentuid;
	int parentgid;
	char path[BUFSIZ];
	char content[BUFSIZ];
	int fd;

	tst_parse_opts(argc, argv, NULL, NULL);
	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		tst_count = 0;
		childpid = ltp_clone_quick(CLONE_NEWUSER | SIGCHLD,
			(void *)child_fn1, NULL);

		if (childpid < 0)
			tst_brkm(TFAIL | TERRNO, cleanup, "clone failed");

		parentuid = geteuid();
		parentgid = getegid();
		sprintf(path, "/proc/%d/uid_map", childpid);
		sprintf(content, "100 %d 1", parentuid);
		fd = SAFE_OPEN(cleanup, path, O_WRONLY, 0644);
		SAFE_WRITE(cleanup, 1, fd, content, strlen(content));
		SAFE_CLOSE(cleanup, fd);

		if (access("/proc/self/setgroups", F_OK) == 0) {
			sprintf(path, "/proc/%d/setgroups", childpid);
			fd = SAFE_OPEN(cleanup, path, O_WRONLY, 0644);
			SAFE_WRITE(cleanup, 1, fd, "deny", 4);
			SAFE_CLOSE(cleanup, fd);
		}

		sprintf(path, "/proc/%d/gid_map", childpid);
		sprintf(content, "100 %d 1", parentgid);
		fd = SAFE_OPEN(cleanup, path, O_WRONLY, 0644);
		SAFE_WRITE(cleanup, 1, fd, content, strlen(content));
		SAFE_CLOSE(cleanup, fd);

		TST_SAFE_CHECKPOINT_WAKE(cleanup, 0);

		tst_record_childstatus(cleanup, childpid);
	}
	cleanup();
	tst_exit();
}
