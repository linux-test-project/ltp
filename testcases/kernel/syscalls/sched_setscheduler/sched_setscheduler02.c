// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2021, BELLSOFT. All rights reserved.
 * Copyright (c) International Business Machines  Corp., 2001
 */

/*\
 * [Description]
 *
 * Testcase to test whether sched_setscheduler(2) sets the errnos
 * correctly.
 *
 * [Algorithm]
 *
 * Call sched_setscheduler as a non-root uid, and expect EPERM
 * to be returned.
 *
 */

#include <stdlib.h>
#include <errno.h>
#include <pwd.h>
#include <sys/types.h>

#include "tst_test.h"
#include "tst_sched.h"

static uid_t nobody_uid;

static void setup(void)
{
	struct passwd *pw = SAFE_GETPWNAM("nobody");

	tst_res(TINFO, "Testing %s variant", sched_variants[tst_variant].desc);

	nobody_uid = pw->pw_uid;
}

static void run(void)
{
	struct sched_variant *tv = &sched_variants[tst_variant];
	pid_t pid = SAFE_FORK();

	if (!pid) {
		struct sched_param p = { .sched_priority = 1 };

		SAFE_SETEUID(nobody_uid);
		TST_EXP_FAIL(tv->sched_setscheduler(0, SCHED_FIFO, &p), EPERM,
			     "sched_setscheduler(0, SCHED_FIFO, %d)",
			     p.sched_priority);
		exit(0);
	}
	tst_reap_children();
}

static struct tst_test test = {
	.forks_child = 1,
	.needs_root = 1,
	.setup = setup,
	.test_variants = ARRAY_SIZE(sched_variants),
	.test_all = run,
};
