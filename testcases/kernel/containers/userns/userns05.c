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
 * A process created via fork(2) or clone(2) without the
 * CLONE_NEWUSER flag is a member of the same user namespace as its
 * parent.
 * When unshare an user namespace, the calling process is moved into
 * a new user namespace which is not shared with any previously
 * existing process.
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

char *TCID = "user_namespace5";
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
	TST_SAFE_CHECKPOINT_WAIT(NULL, 0);
	return 0;
}

static unsigned int getusernsidbypid(int pid)
{
	char path[BUFSIZ];
	char userid[BUFSIZ];
	unsigned int id = 0;

	sprintf(path, "/proc/%d/ns/user", pid);

	if (readlink(path, userid, BUFSIZ) == -1)
		tst_resm(TFAIL | TERRNO, "readlink failure.");

	if (sscanf(userid, "user:[%u]", &id) != 1)
		tst_resm(TFAIL, "sscanf failure.");
	return id;
}

static void test_userns_id(void)
{
	int cpid1, cpid2, cpid3;
	unsigned int parentuserns, cpid1userns, cpid2userns, newparentuserns;

	parentuserns = getusernsidbypid(getpid());
	cpid1 = ltp_clone_quick(SIGCHLD, (void *)child_fn1,
		NULL);
	if (cpid1 < 0)
		tst_brkm(TBROK | TERRNO, cleanup, "clone failed");
	cpid1userns = getusernsidbypid(cpid1);
	TST_SAFE_CHECKPOINT_WAKE(cleanup, 0);

	/* A process created via fork(2) or clone(2) without the
	CLONE_NEWUSER flag is a member of the same user namespace as its
	parent.*/
	if (parentuserns != cpid1userns)
		tst_resm(TFAIL, "userns:parent should be equal to cpid1");

	cpid2 = ltp_clone_quick(CLONE_NEWUSER | SIGCHLD,
		(void *)child_fn1, NULL);
	if (cpid2 < 0)
		tst_brkm(TBROK | TERRNO, cleanup, "clone failed");
	cpid2userns = getusernsidbypid(cpid2);
	TST_SAFE_CHECKPOINT_WAKE(cleanup, 0);

	if (parentuserns == cpid2userns)
		tst_resm(TFAIL, "userns:parent should be not equal to cpid2");

	switch (cpid3 = fork()) {
	case -1:
		tst_brkm(TBROK | TERRNO, cleanup, "fork");
	case 0:
		if (unshare(CLONE_NEWUSER) == -1) {
			printf("parent pid unshare failure: (%d) %s",
				errno, strerror(errno));
			exit(1);
		}
		newparentuserns = getusernsidbypid(getpid());

		/* When unshare an user namespace, the calling process
		is moved into a new user namespace which is not shared
		with any previously existing process.*/
		if (parentuserns == newparentuserns)
			exit(1);
		exit(0);
	}

	tst_record_childstatus(cleanup, cpid1);
	tst_record_childstatus(cleanup, cpid2);
	tst_record_childstatus(cleanup, cpid3);
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

	tst_parse_opts(argc, argv, NULL, NULL);
	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		tst_count = 0;
		test_userns_id();
	}
	cleanup();
	tst_exit();
}
