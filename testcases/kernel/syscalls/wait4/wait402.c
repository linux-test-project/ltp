// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines  Corp., 2001
 * Copyright (c) 2012 Cyril Hrubis <chrubis@suse.cz>
 * Copyright (c) Linux Test Project, 2001-2015
 * Copyright (c) 2023 Ioannis Bonatakis <ybonatakis@suse.com>
 */

/*\
 * Check for ECHILD errno when call wait4(2) with an invalid pid value.
 */

#include "tst_test.h"
#include <sys/wait.h>

static pid_t pid_max;

static void run(void)
{
	int status;
	struct rusage rusage;

	TST_EXP_FAIL2(wait4(pid_max + 1, &status, 0, &rusage), ECHILD);
}

static void setup(void)
{
	SAFE_FILE_SCANF("/proc/sys/kernel/pid_max", "%d\n", &pid_max);
}

static struct tst_test test = {
	.test_all = run,
	.setup = setup,
};
