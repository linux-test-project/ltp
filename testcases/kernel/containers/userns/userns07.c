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
 * The kernel imposes a limit of at least 32 nested levels on user namespaces.
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

#define MAXNEST 32

char *TCID = "userns07";
int TST_TOTAL = 1;

static void setup(void)
{
	check_newuser();
	tst_tmpdir();
	TST_CHECKPOINT_INIT(NULL);
}

static void cleanup(void)
{
	tst_rmdir();
}

static int child_fn1(void *arg)
{
	pid_t cpid1;
	long level = (long)arg;
	int status;
	int parentuid;
	int parentgid;

	TST_SAFE_CHECKPOINT_WAIT(NULL, 0);

	if (level == MAXNEST)
		return 0;
	cpid1 = ltp_clone_quick(CLONE_NEWUSER | SIGCHLD,
		(void *)child_fn1, (void *)(level + 1));
	if (cpid1 < 0) {
		printf("level %ld:unexpected error: (%d) %s\n",
			level, errno, strerror(errno));
		return 1;
	}

	parentuid = geteuid();
	parentgid = getegid();

	updatemap(cpid1, UID_MAP, 0, parentuid, NULL);
	updatemap(cpid1, GID_MAP, 0, parentgid, NULL);

	TST_SAFE_CHECKPOINT_WAKE(cleanup, 0);

	if (waitpid(cpid1, &status, 0) == -1)
		return 1;

	if (WIFEXITED(status) && WEXITSTATUS(status) != 0) {
		printf("child exited abnormally\n");
		return 1;
	} else if (WIFSIGNALED(status)) {
		printf("child was killed with signal = %d", WTERMSIG(status));
		return 1;
	}
	return 0;
}

static void test_max_nest(void)
{
	pid_t cpid1;
	int parentuid;
	int parentgid;
	int fd;
	char path[BUFSIZ];

	cpid1 = ltp_clone_quick(CLONE_NEWUSER | SIGCHLD,
		(void *)child_fn1, (void *)0);
	if (cpid1 < 0)
		tst_brkm(TBROK | TERRNO, cleanup, "clone failed");

	parentuid = geteuid();
	parentgid = getegid();

	if (access("/proc/self/setgroups", F_OK) == 0) {
		sprintf(path, "/proc/%d/setgroups", cpid1);
		fd = SAFE_OPEN(cleanup, path, O_WRONLY, 0644);
		SAFE_WRITE(cleanup, 1, fd, "deny", 4);
		SAFE_CLOSE(cleanup, fd);
	}

	updatemap(cpid1, UID_MAP, 0, parentuid, cleanup);
	updatemap(cpid1, GID_MAP, 0, parentgid, cleanup);

	TST_SAFE_CHECKPOINT_WAKE(cleanup, 0);
	tst_record_childstatus(cleanup, cpid1);
}

int main(int argc, char *argv[])
{
	int lc;

	setup();
	tst_parse_opts(argc, argv, NULL, NULL);

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		tst_count = 0;
		test_max_nest();
	}

	cleanup();
	tst_exit();
}

