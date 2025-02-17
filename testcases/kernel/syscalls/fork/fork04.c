// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2000 Silicon Graphics, Inc.  All Rights Reserved.
 * Copyright (c) Linux Test Project, 2001-2023
 * Copyright (C) 2023 SUSE LLC Andrea Cervesato <andrea.cervesato@suse.com>
 */

/*\
 * This test verifies that parent process shares environ variables with the
 * child and that child doesn't change parent's environ variables.
 */

#include <stdlib.h>
#include "tst_test.h"

#define ENV_KEY "LTP_FORK04"
#define ENV_VAL0 "PASS"
#define ENV_VAL1 "FAIL"

static void run_child(void)
{
	const char *val;

	val = getenv(ENV_KEY);
	if (!val)
		tst_brk(TBROK, "Can't find %s environ variable", ENV_KEY);

	TST_EXP_EXPR(strcmp(ENV_VAL0, val) == 0,
		"%s environ variable has been inherited by the child",
		ENV_KEY);

	tst_res(TINFO, "Unset %s environ variable inside child", ENV_KEY);

	if (unsetenv(ENV_KEY) == -1)
		tst_brk(TBROK, "Can't unset %s environ variable", ENV_KEY);

	TST_CHECKPOINT_WAKE_AND_WAIT(0);

	tst_res(TINFO, "Set %s=%s environ variable inside child", ENV_KEY, ENV_VAL1);

	SAFE_SETENV(ENV_KEY, ENV_VAL1, 0);

	TST_CHECKPOINT_WAKE(0);
}

static void run(void)
{
	const char *val;

	tst_res(TINFO,
		"Set %s=%s environ variable inside parent",
		ENV_KEY, ENV_VAL0);

	SAFE_SETENV(ENV_KEY, ENV_VAL0, 0);

	tst_res(TINFO, "Spawning child");

	if (!SAFE_FORK()) {
		run_child();
		exit(0);
	}

	TST_CHECKPOINT_WAIT(0);

	val = getenv(ENV_KEY);
	if (!val) {
		tst_res(TFAIL,
			"%s environ variable has been unset inside parent",
			ENV_KEY);
	} else {
		TST_EXP_EXPR(strcmp(ENV_VAL0, val) == 0,
			"%s environ variable is still present inside parent",
			ENV_KEY);
	}

	TST_CHECKPOINT_WAKE_AND_WAIT(0);

	val = getenv(ENV_KEY);
	if (!val)
		tst_res(TFAIL,
			"%s environ variable has been unset inside parent",
			ENV_KEY);
	else {
		TST_EXP_EXPR(strcmp(ENV_VAL0, val) == 0,
			"%s environ variable didn't change inside parent",
			ENV_KEY);
	}
}

static struct tst_test test = {
	.test_all = run,
	.forks_child = 1,
	.needs_checkpoints = 1,
};
