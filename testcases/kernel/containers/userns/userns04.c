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
 *  If a namespace isn't another namespace's ancestor, the process in
 *  first namespace does not have the CAP_SYS_ADMIN capability in the
 *  second namespace and the setns() call fails.
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

char *TCID = "user_namespace4";
int TST_TOTAL = 1;

static void setup(void)
{
	check_newuser();
	tst_syscall(__NR_setns, -1, 0);
	tst_tmpdir();
	TST_CHECKPOINT_INIT(NULL);
}

static void cleanup(void)
{
	tst_rmdir();
}

static int child_fn1(void *arg LTP_ATTRIBUTE_UNUSED)
{
	TST_SAFE_CHECKPOINT_WAIT(NULL, 0);
	return 0;
}

static int child_fn2(void *arg)
{
	int exit_val = 0;
	int ret;

	ret = tst_syscall(__NR_setns, ((long)arg), CLONE_NEWUSER);
	if (ret != -1) {
		printf("child2 setns() unexpected success\n");
		exit_val = 1;
	} else if (errno != EPERM) {
		printf("child2 setns() unexpected error: (%d) %s\n",
			errno, strerror(errno));
		exit_val = 1;
	}

	TST_SAFE_CHECKPOINT_WAIT(NULL, 1);
	return exit_val;
}

static void test_cap_sys_admin(void)
{
	pid_t cpid1, cpid2, cpid3;
	char path[BUFSIZ];
	int fd;

	/* child 1 */
	cpid1 = ltp_clone_quick(CLONE_NEWUSER | SIGCHLD,
		(void *)child_fn1, NULL);
	if (cpid1 < 0)
		tst_brkm(TBROK | TERRNO, cleanup, "clone failed");

	/* child 2 */
	sprintf(path, "/proc/%d/ns/user", cpid1);
	fd = SAFE_OPEN(cleanup, path, O_RDONLY, 0644);
	cpid2 = ltp_clone_quick(CLONE_NEWUSER | SIGCHLD,
		(void *)child_fn2, (void *)((long)fd));
	if (cpid2 < 0)
		tst_brkm(TBROK | TERRNO, cleanup, "clone failed");

	/* child 3 - throw-away process changing ns to child1 */
	switch (cpid3 = fork()) {
	case -1:
		tst_brkm(TBROK | TERRNO, cleanup, "fork");
	case 0:
		if (tst_syscall(__NR_setns, fd, CLONE_NEWUSER) == -1) {
			printf("parent pid setns failure: (%d) %s",
				errno, strerror(errno));
			exit(1);
		}
		exit(0);
	}

	TST_SAFE_CHECKPOINT_WAKE(cleanup, 0);
	TST_SAFE_CHECKPOINT_WAKE(cleanup, 1);

	tst_record_childstatus(cleanup, cpid1);
	tst_record_childstatus(cleanup, cpid2);
	tst_record_childstatus(cleanup, cpid3);

	SAFE_CLOSE(cleanup, fd);

}

int main(int argc, char *argv[])
{
	int lc;

	setup();
	tst_parse_opts(argc, argv, NULL, NULL);

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		tst_count = 0;
		test_cap_sys_admin();
	}

	cleanup();
	tst_exit();
}
