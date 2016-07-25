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
 * /proc/PID/uid_map and /proc/PID/gid_map contains three values separated by
 * white space:
 * ID-inside-ns   ID-outside-ns   length
 *
 * ID-outside-ns is interpreted according to which process is opening the file.
 * If the process opening the file is in the same user namespace as the process
 * PID, then ID-outside-ns is defined with respect to the parent user namespace.
 * If the process opening the file is in a different user namespace, then
 * ID-outside-ns is defined with respect to the user namespace of the process
 * opening the file.
 *
 * The string "deny" would be written to /proc/self/setgroups before GID
 * check if setgroups is allowed, see kernel commits:
 *
 *   commit 9cc46516ddf497ea16e8d7cb986ae03a0f6b92f8
 *   Author: Eric W. Biederman <ebiederm@xmission.com>
 *   Date:   Tue Dec 2 12:27:26 2014 -0600
 *     userns: Add a knob to disable setgroups on a per user namespace basis
 *
 *   commit 66d2f338ee4c449396b6f99f5e75cd18eb6df272
 *   Author: Eric W. Biederman <ebiederm@xmission.com>
 *   Date:   Fri Dec 5 19:36:04 2014 -0600
 *     userns: Allow setting gid_maps without privilege when setgroups is disabled
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
#include "userns_helper.h"
#include "test.h"

#define CHILD1UID 0
#define CHILD1GID 0
#define CHILD2UID 200
#define CHILD2GID 200
#define UID_MAP 0
#define GID_MAP 1

char *TCID = "user_namespace3";
int TST_TOTAL = 1;
static int cpid1, parentuid, parentgid;

/*
 * child_fn1() - Inside a new user namespace
 */
static int child_fn1(void)
{
	TST_SAFE_CHECKPOINT_WAIT(NULL, 0);
	return 0;
}

/*
 * child_fn2() - Inside a new user namespace
 */
static int child_fn2(void)
{
	int exit_val = 0;
	int uid, gid;
	char cpid1uidpath[BUFSIZ];
	char cpid1gidpath[BUFSIZ];
	int idinsidens, idoutsidens, length;

	TST_SAFE_CHECKPOINT_WAIT(NULL, 1);

	uid = geteuid();
	gid = getegid();

	if (uid != CHILD2UID || gid != CHILD2GID) {
		printf("unexpected uid=%d gid=%d\n", uid, gid);
		exit_val = 1;
	}

	/*Get the uid parameters of the child_fn2 process.*/
	SAFE_FILE_SCANF(NULL, "/proc/self/uid_map", "%d %d %d", &idinsidens,
		&idoutsidens, &length);

	/* map file format:ID-inside-ns   ID-outside-ns   length
	If the process opening the file is in the same user namespace as
	the process PID, then ID-outside-ns is defined with respect to the
	 parent user namespace.*/
	if (idinsidens != CHILD2UID || idoutsidens != parentuid) {
		printf("child_fn2 checks /proc/cpid2/uid_map:\n");
		printf("unexpected: idinsidens=%d idoutsidens=%d\n",
			idinsidens, idoutsidens);
		exit_val = 1;
	}

	sprintf(cpid1uidpath, "/proc/%d/uid_map", cpid1);
	SAFE_FILE_SCANF(NULL, cpid1uidpath, "%d %d %d", &idinsidens,
		&idoutsidens, &length);

	/* If the process opening the file is in a different user namespace,
	then ID-outside-ns is defined with respect to the user namespace
	of the process opening the file.*/
	if (idinsidens != CHILD1UID || idoutsidens != CHILD2UID) {
		printf("child_fn2 checks /proc/cpid1/uid_map:\n");
		printf("unexpected: idinsidens=%d idoutsidens=%d\n",
			idinsidens, idoutsidens);
		exit_val = 1;
	}

	sprintf(cpid1gidpath, "/proc/%d/gid_map", cpid1);
	SAFE_FILE_SCANF(NULL, "/proc/self/gid_map", "%d %d %d",
		 &idinsidens, &idoutsidens, &length);

	if (idinsidens != CHILD2GID || idoutsidens != parentgid) {
		printf("child_fn2 checks /proc/cpid2/gid_map:\n");
		printf("unexpected: idinsidens=%d idoutsidens=%d\n",
			idinsidens, idoutsidens);
		exit_val = 1;
	}

	SAFE_FILE_SCANF(NULL, cpid1gidpath, "%d %d %d", &idinsidens,
		&idoutsidens, &length);

	if (idinsidens != CHILD1GID || idoutsidens != CHILD2GID) {
		printf("child_fn1 checks /proc/cpid1/gid_map:\n");
		printf("unexpected: idinsidens=%d idoutsidens=%d\n",
			idinsidens, idoutsidens);
		exit_val = 1;
	}

	TST_SAFE_CHECKPOINT_WAKE(NULL, 0);
	TST_SAFE_CHECKPOINT_WAKE(NULL, 1);
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
}

int main(int argc, char *argv[])
{
	pid_t cpid2;
	char path[BUFSIZ];
	int lc;
	int fd;
	int ret;

	tst_parse_opts(argc, argv, NULL, NULL);
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
			/* If the setgroups file has the value "deny",
			 * then the setgroups(2) system call can't
			 * subsequently be reenabled (by writing "allow" to
			 * the file) in this user namespace.  (Attempts to
			 * do so will fail with the error EPERM.)
			*/

			/* test that setgroups can't be re-enabled */
			fd = SAFE_OPEN(cleanup, path, O_WRONLY, 0644);
			ret = write(fd, "allow", 5);

			if (ret != -1) {
				tst_brkm(TBROK | TERRNO, cleanup,
					"write action should fail");
			} else if (errno != EPERM) {
				tst_brkm(TBROK | TERRNO, cleanup,
					"unexpected error: \n");
			}
			SAFE_CLOSE(cleanup, fd);
			tst_resm(TPASS, "setgroups can't be re-enabled");

			sprintf(path, "/proc/%d/setgroups", cpid2);
			fd = SAFE_OPEN(cleanup, path, O_WRONLY, 0644);
			SAFE_WRITE(cleanup, 1, fd, "deny", 4);
			SAFE_CLOSE(cleanup, fd);
		}

		updatemap(cpid1, UID_MAP, CHILD1UID, parentuid, cleanup);
		updatemap(cpid2, UID_MAP, CHILD2UID, parentuid, cleanup);

		updatemap(cpid1, GID_MAP, CHILD1GID, parentgid, cleanup);
		updatemap(cpid2, GID_MAP, CHILD2GID, parentgid, cleanup);

		TST_SAFE_CHECKPOINT_WAKE_AND_WAIT(cleanup, 1);

		tst_record_childstatus(cleanup, cpid1);
		tst_record_childstatus(cleanup, cpid2);
	}
	cleanup();
	tst_exit();
}
