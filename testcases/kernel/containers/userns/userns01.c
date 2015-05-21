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
 *  If a user ID has no mapping inside the namespace, user ID and group
 * ID will be the value defined in the file /proc/sys/kernel/overflowuid, 65534.
 */

#define _GNU_SOURCE
#include <sys/wait.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include "test.h"
#include "libclone.h"
#include "userns_helper.h"
#define OVERFLOWUIDPATH "/proc/sys/kernel/overflowuid"

char *TCID = "user_namespace1";
int TST_TOTAL = 1;

char fullpath[BUFSIZ];
long overflowuid;

/*
 * child_fn1() - Inside a new user namespace
 */
static int child_fn1(void *arg)
{
	int exit_val;
	int uid, gid;

	uid = geteuid();
	gid = getegid();

	tst_resm(TINFO, "USERNS test is running in a new user namespace.");
	if (uid == overflowuid && gid == overflowuid) {
		printf("Got expected uid and gid\n");
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
	SAFE_FILE_SCANF(NULL, OVERFLOWUIDPATH, "%ld", &overflowuid);
}

int main(int argc, char *argv[])
{
	int status;

	tst_parse_opts(argc, argv, NULL, NULL);
	setup();

	TEST(do_clone_unshare_test(T_CLONE, CLONE_NEWUSER, child_fn1, NULL));

	if (TEST_RETURN == -1)
		tst_brkm(TFAIL | TTERRNO, NULL, "clone failed");
	else if ((wait(&status)) == -1)
		tst_brkm(TWARN | TERRNO, NULL, "wait failed");

	if (WIFEXITED(status) && WEXITSTATUS(status) != 0)
		tst_resm(TFAIL, "child exited abnormally");
	else if (WIFSIGNALED(status)) {
		tst_resm(TFAIL, "child was killed with signal = %d",
			 WTERMSIG(status));
	}

	tst_resm(TPASS, "the uid and the gid are right inside the container");
	tst_exit();
}

