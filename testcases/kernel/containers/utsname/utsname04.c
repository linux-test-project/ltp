// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2023 SUSE LLC Andrea Cervesato <andrea.cervesato@suse.com>
 */

/*\
 * [Description]
 *
 * Drop root privileges, create a container with CLONE_NEWUTS and verify that
 * we receive a permission error.
 */

#define _GNU_SOURCE

#include <pwd.h>
#include "tst_test.h"
#include "lapi/sched.h"

static char *str_op;

static void run(void)
{
	const struct tst_clone_args cargs = { CLONE_NEWUTS, SIGCHLD };
	struct passwd *pw;

	tst_res(TINFO, "Dropping root privileges");

	pw = SAFE_GETPWNAM("nobody");
	SAFE_SETRESUID(pw->pw_uid, pw->pw_uid, pw->pw_uid);

	if (!str_op || !strcmp(str_op, "clone")) {
		TST_EXP_EQ_LI(tst_clone(&cargs), -1);
		TST_EXP_PASS(errno == EPERM);
	} else {
		if (!SAFE_FORK()) {
			TST_EXP_EQ_LI(unshare(CLONE_NEWUTS), -1);
			TST_EXP_PASS(errno == EPERM);
			return;
		}
	}
}

static struct tst_test test = {
	.test_all = run,
	.needs_root = 1,
	.forks_child = 1,
	.needs_checkpoints = 1,
	.options = (struct tst_option[]) {
		{ "m:", &str_op, "Test execution mode <clone|unshare>" },
		{},
	},
};
