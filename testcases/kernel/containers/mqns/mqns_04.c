// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines Corp., 2009
 * Copyright (c) Serge Hallyn <serue@us.ibm.com>
 * Copyright (C) 2023 SUSE LLC Andrea Cervesato <andrea.cervesato@suse.com>
 */

/*\
 * Test mqueuefs manipulation from child/parent namespaces.
 *
 * [Algorithm]
 *
 * - parent creates mqueue folder in <tmpdir>
 * - child mq_open() /MQ1 mqueue
 * - child mounts mqueue there
 * - parent checks for <tmpdir>/mqueue/MQ1 existence
 * - child exits
 * - parent checks for <tmpdir>/mqueue/MQ1 existence
 * - parent tries 'touch <tmpdir>/mqueue/MQ2' -> should fail
 * - parent umount mqueuefs
 */

#include <sys/wait.h>
#include "tst_test.h"
#include "lapi/sched.h"
#include "tst_safe_posix_ipc.h"
#include "tst_safe_stdio.h"
#include "tst_safe_macros.h"

#define CHECK_MQ_OPEN_RET(x) ((x) >= 0 || ((x) == -1 && errno != EMFILE))

#define DEVDIR "ltp_mqueue"
#define MQNAME1 "/MQ1"
#define MQNAME2 "/MQ2"
#define MQUEUE1 DEVDIR MQNAME1
#define MQUEUE2 DEVDIR MQNAME2

static char *str_op;

static void check_mqueue(void)
{
	mqd_t mqd;

	tst_res(TINFO, "Creating %s mqueue from within child process", MQNAME1);

	mqd = TST_RETRY_FUNC(
		mq_open(MQNAME1, O_RDWR | O_CREAT | O_EXCL, 0755, NULL),
		CHECK_MQ_OPEN_RET);
	if (mqd == -1)
		tst_brk(TBROK | TERRNO, "mq_open failed");

	SAFE_MQ_CLOSE(mqd);

	tst_res(TINFO, "Mount %s from within child process", DEVDIR);

	SAFE_MOUNT("mqueue", DEVDIR, "mqueue", 0, NULL);

	TST_CHECKPOINT_WAKE_AND_WAIT(0);
}

static void run(void)
{
	const struct tst_clone_args clone_args = {
		.flags = CLONE_NEWIPC,
		.exit_signal = SIGCHLD
	};

	if (str_op && !strcmp(str_op, "clone")) {
		tst_res(TINFO, "Spawning isolated process");

		if (!SAFE_CLONE(&clone_args)) {
			check_mqueue();
			return;
		}
	} else if (str_op && !strcmp(str_op, "unshare")) {
		tst_res(TINFO, "Spawning unshared process");

		if (!SAFE_FORK()) {
			SAFE_UNSHARE(CLONE_NEWIPC);
			check_mqueue();
			return;
		}
	}

	TST_CHECKPOINT_WAIT(0);

	if (access(MQUEUE1, F_OK))
		tst_res(TFAIL, MQUEUE1 " can't be accessed from parent");
	else
		tst_res(TPASS, MQUEUE1 " can be accessed from parent");

	TST_CHECKPOINT_WAKE(0);

	tst_res(TINFO, "Waiting child to exit");

	tst_reap_children();

	if (access(MQUEUE1, F_OK))
		tst_res(TFAIL, MQUEUE1 " can't be accessed from parent");
	else
		tst_res(TPASS, MQUEUE1 " can be accessed from parent");

	tst_res(TINFO, "Try to create %s from parent", MQUEUE2);

	TST_EXP_FAIL(creat(MQUEUE2, 0755), EACCES);

	SAFE_UMOUNT(DEVDIR);
}

static void setup(void)
{
	if (!str_op || (strcmp(str_op, "clone") && strcmp(str_op, "unshare")))
		tst_brk(TCONF, "Please specify clone|unshare child isolation");

	SAFE_MKDIR(DEVDIR, 0755);
}

static void cleanup(void)
{
	if (!access(MQUEUE1, F_OK))
		SAFE_MQ_UNLINK(MQNAME1);

	if (!access(MQUEUE2, F_OK))
		SAFE_MQ_UNLINK(MQNAME2);

	if (tst_is_mounted(DEVDIR))
		SAFE_UMOUNT(DEVDIR);
}

static struct tst_test test = {
	.test_all = run,
	.setup = setup,
	.cleanup = cleanup,
	.needs_root = 1,
	.forks_child = 1,
	.needs_checkpoints = 1,
	.options = (struct tst_option[]) {
		{ "m:", &str_op, "Child process isolation <clone|unshare>" },
		{},
	},
	.needs_kconfigs = (const char *[]) {
		"CONFIG_USER_NS",
		NULL
	},
};
