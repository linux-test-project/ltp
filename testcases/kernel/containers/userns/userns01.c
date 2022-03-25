// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) Huawei Technologies Co., Ltd., 2015
 * Copyright (C) 2022 SUSE LLC Andrea Cervesato <andrea.cervesato@suse.com>
 */

/*\
 * [Description]
 *
 * Verify that if a user ID has no mapping inside the namespace, user ID and
 * group ID will be the value defined in the file /proc/sys/kernel/overflowuid
 * (defaults to 65534) and /proc/sys/kernel/overflowgid (defaults to 65534). A
 * child process has a full set of permitted and effective capabilities, even
 * though the program was run from an unprivileged account.
 */

#include "tst_test.h"

#ifdef HAVE_LIBCAP
#define _GNU_SOURCE

#include <stdio.h>
#include "common.h"
#include "config.h"
#include <sys/capability.h>

#define OVERFLOWUIDPATH "/proc/sys/kernel/overflowuid"
#define OVERFLOWGIDPATH "/proc/sys/kernel/overflowgid"

static long overflowuid;
static long overflowgid;

/*
 * child_fn1() - Inside a new user namespace
 */
static int child_fn1(LTP_ATTRIBUTE_UNUSED void *arg)
{
	int uid, gid;
	cap_t caps;
	int i, last_cap;
	cap_flag_value_t flag_val;

	uid = geteuid();
	gid = getegid();

	tst_res(TINFO, "USERNS test is running in a new user namespace.");

	if (uid != overflowuid || gid != overflowgid)
		tst_res(TFAIL, "got unexpected uid=%d gid=%d", uid, gid);
	else
		tst_res(TPASS, "got expected uid and gid");

	caps = cap_get_proc();

	SAFE_FILE_SCANF("/proc/sys/kernel/cap_last_cap", "%d", &last_cap);

	for (i = 0; i <= last_cap; i++) {
		cap_get_flag(caps, i, CAP_EFFECTIVE, &flag_val);
		if (!flag_val)
			break;

		cap_get_flag(caps, i, CAP_PERMITTED, &flag_val);
		if (!flag_val)
			break;
	}

	if (!flag_val)
		tst_res(TFAIL, "unexpected effective/permitted caps at %d", i);
	else
		tst_res(TPASS, "expected capabilities");

	return 0;
}

static void setup(void)
{
	check_newuser();

	SAFE_FILE_SCANF(OVERFLOWUIDPATH, "%ld", &overflowuid);
	SAFE_FILE_SCANF(OVERFLOWGIDPATH, "%ld", &overflowgid);
}

static void run(void)
{
	int pid;

	pid = ltp_clone_quick(CLONE_NEWUSER | SIGCHLD, child_fn1, NULL);
	if (pid < 0)
		tst_brk(TBROK | TTERRNO, "clone failed");
}

static struct tst_test test = {
	.setup = setup,
	.test_all = run,
	.needs_root = 1,
	.caps = (struct tst_cap []) {
		TST_CAP(TST_CAP_DROP, CAP_NET_RAW),
		{}
	},
	.needs_kconfigs = (const char *[]) {
		"CONFIG_USER_NS",
		NULL,
	},
};

#else
TST_TEST_TCONF("System is missing libcap");
#endif
