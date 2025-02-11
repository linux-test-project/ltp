// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines Corp., 2009
 * Copyright (c) Nadia Derbey, 2009 <Nadia.Derbey@bull.net>
 * Copyright (C) 2023 SUSE LLC Andrea Cervesato <andrea.cervesato@suse.com>
 */

/*\
 * Create a mqueue with the same name in both parent and isolated/forked child,
 * then check namespace isolation.
 */

#include "tst_test.h"
#include "lapi/sched.h"
#include "tst_safe_posix_ipc.h"

#define MQNAME "/MQ1"

static mqd_t mqd;
static char *str_op;

static int create_message_queue(void)
{
	return mq_open(MQNAME, O_RDWR | O_CREAT | O_EXCL, 0777, NULL);
}

static void shared_child(void)
{
	mqd_t mqd1 = -1;

	TST_EXP_FAIL(mqd1 = create_message_queue(), EEXIST);

	if (mqd1 != -1) {
		SAFE_MQ_CLOSE(mqd1);
		SAFE_MQ_UNLINK(MQNAME);
	}
}

static void isolated_child(void)
{
	mqd_t mqd1 = -1;

	TST_EXP_POSITIVE(mqd1 = create_message_queue());

	if (mqd1 != -1) {
		SAFE_MQ_CLOSE(mqd1);
		SAFE_MQ_UNLINK(MQNAME);
	}
}

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
			isolated_child();
			return;
		}
	} else if (str_op && !strcmp(str_op, "unshare")) {
		tst_res(TINFO, "Spawning unshared process");

		if (!SAFE_FORK()) {
			SAFE_UNSHARE(CLONE_NEWIPC);
			isolated_child();
			return;
		}
	} else {
		tst_res(TINFO, "Spawning plain process");

		if (!SAFE_FORK()) {
			shared_child();
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
