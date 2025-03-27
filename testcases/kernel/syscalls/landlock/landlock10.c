// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2025 SUSE LLC Andrea Cervesato <andrea.cervesato@suse.com>
 */

/*\
 * Verify that landlock's LANDLOCK_SCOPE_SIGNAL rule rejects any signal coming
 * from a process on a different domain, but accept signals from processes in
 * the same domain.
 */

#include "tst_test.h"
#include "landlock_common.h"

static struct tst_landlock_ruleset_attr_abi6 *ruleset_attr;

enum {
	DOMAIN_PAUSED = 0,
	DOMAIN_KILLER,
	DOMAIN_BOTH,
};

static void scoped_sandbox(const char *from)
{
	tst_res(TINFO, "Enforcing rule LANDLOCK_SCOPE_SIGNAL for %s process", from);

	ruleset_attr->scoped = LANDLOCK_SCOPE_SIGNAL;
	apply_landlock_scoped_layer(ruleset_attr, sizeof(*ruleset_attr));
}

static void run(void)
{
	/* isolate test inside a process so we won't stack too many
	 * layers (-E2BIG) when there are multiple test's iterations
	 */
	if (SAFE_FORK())
		return;

	if (tst_variant == DOMAIN_BOTH)
		scoped_sandbox("paused and killer");

	pid_t paused_pid;
	pid_t killer_pid;

	paused_pid = SAFE_FORK();
	if (!paused_pid) {
		if (tst_variant == DOMAIN_PAUSED)
			scoped_sandbox("paused");

		TST_CHECKPOINT_WAKE(0);
		pause();
		exit(0);
	}

	TST_CHECKPOINT_WAIT(0);
	TST_PROCESS_STATE_WAIT(paused_pid, 'S', 10000);

	killer_pid = SAFE_FORK();
	if (!killer_pid) {
		if (tst_variant == DOMAIN_KILLER)
			scoped_sandbox("killer");

		TST_CHECKPOINT_WAKE(0);

		if (tst_variant == DOMAIN_KILLER)
			TST_EXP_FAIL(kill(paused_pid, SIGKILL), EPERM);
		else
			TST_EXP_PASS(kill(paused_pid, SIGKILL));

		exit(0);
	}

	TST_CHECKPOINT_WAIT(0);
	SAFE_WAITPID(killer_pid, NULL, 0);

	if (kill(paused_pid, SIGKILL) == -1) {
		if (errno != ESRCH)
			tst_brk(TBROK | TERRNO, "kill(%u, SIGKILL) error", paused_pid);
	}

	SAFE_WAITPID(paused_pid, NULL, 0);
}

static void setup(void)
{
	int abi;

	abi = verify_landlock_is_enabled();
	if (abi < 6)
		tst_brk(TCONF, "LANDLOCK_SCOPE_SIGNAL is unsupported on ABI < 6");
}

static struct tst_test test = {
	.test_all = run,
	.setup = setup,
	.needs_root = 1,
	.forks_child = 1,
	.needs_checkpoints = 1,
	.test_variants = 3,
	.bufs = (struct tst_buffers []) {
		{&ruleset_attr, .size = sizeof(struct tst_landlock_ruleset_attr_abi6)},
		{},
	},
	.caps = (struct tst_cap []) {
		TST_CAP(TST_CAP_REQ, CAP_SYS_ADMIN),
		{}
	},
};
