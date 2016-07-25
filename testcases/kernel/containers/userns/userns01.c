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
 * ID will be the value defined in the file /proc/sys/kernel/overflowuid(65534)
 * and /proc/sys/kernel/overflowgid(65534). A child process has a full set
 * of permitted and effective capabilities, even though the program was
 * run from an unprivileged account.
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
#include "config.h"
#if HAVE_SYS_CAPABILITY_H
#include <sys/capability.h>
#endif

#define OVERFLOWUIDPATH "/proc/sys/kernel/overflowuid"
#define OVERFLOWGIDPATH "/proc/sys/kernel/overflowgid"

char *TCID = "user_namespace1";
int TST_TOTAL = 1;

static long overflowuid;
static long overflowgid;

/*
 * child_fn1() - Inside a new user namespace
 */
static int child_fn1(void *arg LTP_ATTRIBUTE_UNUSED)
{
	int exit_val = 0;
	int uid, gid;
#ifdef HAVE_LIBCAP
	cap_t caps;
	int i, last_cap;
	cap_flag_value_t flag_val;
#endif

	uid = geteuid();
	gid = getegid();

	tst_resm(TINFO, "USERNS test is running in a new user namespace.");

	if (uid != overflowuid || gid != overflowgid) {
		printf("Got unexpected result of uid=%d gid=%d\n", uid, gid);
		exit_val = 1;
	}

#ifdef HAVE_LIBCAP
	caps = cap_get_proc();
	SAFE_FILE_SCANF(NULL, "/proc/sys/kernel/cap_last_cap", "%d", &last_cap);
	for (i = 0; i <= last_cap; i++) {
		cap_get_flag(caps, i, CAP_EFFECTIVE, &flag_val);
		if (flag_val == 0)
			break;
		cap_get_flag(caps, i, CAP_PERMITTED, &flag_val);
		if (flag_val == 0)
			break;
	}

	if (flag_val == 0) {
		printf("unexpected effective/permitted caps at %d\n", i);
		exit_val = 1;
	}
#else
	printf("System is missing libcap.\n");
#endif
	return exit_val;
}

static void setup(void)
{
	check_newuser();
	SAFE_FILE_SCANF(NULL, OVERFLOWUIDPATH, "%ld", &overflowuid);
	SAFE_FILE_SCANF(NULL, OVERFLOWGIDPATH, "%ld", &overflowgid);
}

int main(int argc, char *argv[])
{
	int lc;

	tst_parse_opts(argc, argv, NULL, NULL);
	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		TEST(do_clone_unshare_test(T_CLONE, CLONE_NEWUSER,
			child_fn1, NULL));

		if (TEST_RETURN == -1)
			tst_brkm(TFAIL | TTERRNO, NULL, "clone failed");
		tst_record_childstatus(NULL, -1);
	}
	tst_exit();
}
