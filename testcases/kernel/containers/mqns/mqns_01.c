// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines Corp., 2009
 * Copyright (c) Nadia Derbey, 2009 <Nadia.Derbey@bull.net>
 * Copyright (C) 2023 SUSE LLC Andrea Cervesato <andrea.cervesato@suse.com>
 */

/*\
 * [Description]
 *
 * Create a mqueue inside the parent and check if it can be accessed from
 * the child namespace. Isolated and unshared process can't access to parent,
 * but plain process can.
 */

#include "tst_test.h"
#include "lapi/sched.h"
#include "tst_safe_posix_ipc.h"

#define MQNAME "/MQ1"

static mqd_t mqd;
static char *str_op;

static void run(void)
{
	const struct tst_clone_args clone_args = {
		.flags = CLONE_NEWIPC,
		.exit_signal = SIGCHLD,
	};

	tst_res(TINFO, "Checking namespaces isolation from parent to child");

	if (str_op && !strcmp(str_op, "clone")) {
		tst_res(TINFO, "Spawning isolated process");

		if (!SAFE_CLONE(&clone_args)) {
			TST_EXP_FAIL(mq_open(MQNAME, O_RDONLY), ENOENT);
			return;
		}
	} else if (str_op && !strcmp(str_op, "unshare")) {
		tst_res(TINFO, "Spawning unshared process");

		if (!SAFE_FORK()) {
			SAFE_UNSHARE(CLONE_NEWIPC);
			TST_EXP_FAIL(mq_open(MQNAME, O_RDONLY), ENOENT);
			return;
		}
	} else {
		tst_res(TINFO, "Spawning plain process");

		if (!SAFE_FORK()) {
			TST_EXP_POSITIVE(mq_open(MQNAME, O_RDONLY));
			return;
		}
	}
}

static void setup(void)
{
	mqd = SAFE_MQ_OPEN(MQNAME, O_RDWR | O_CREAT | O_EXCL, 0777, NULL);
}

static void cleanup(void)
{
	if (mqd != -1) {
		SAFE_MQ_CLOSE(mqd);
		SAFE_MQ_UNLINK(MQNAME);
	}
}

static struct tst_test test = {
	.test_all = run,
	.setup = setup,
	.cleanup = cleanup,
	.needs_root = 1,
	.forks_child = 1,
	.options = (struct tst_option[]) {
		{ "m:", &str_op, "Child process isolation <clone|unshare>" },
		{},
	},
	.needs_kconfigs = (const char *[]) {
		"CONFIG_USER_NS",
		NULL
	},
};
