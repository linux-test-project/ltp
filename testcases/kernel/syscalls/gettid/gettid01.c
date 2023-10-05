// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2007-2008, Hitachi, Ltd.
 * Copyright (C) 2023 SUSE LLC Andrea Cervesato <andrea.cervesato@suse.com>
 */
/*\
 * [Description]
 *
 * This test checks if parent pid is equal to tid in single-threaded
 * application.
 */

#include "tst_test.h"
#include "lapi/syscalls.h"

static void run(void)
{
	long pid, tid;

	SAFE_FILE_LINES_SCANF("/proc/self/status", "Pid: %ld", &pid);
	SAFE_FILE_LINES_SCANF("/proc/self/status", "Tgid: %ld", &tid);

	if (pid != tid)
		tst_brk(TBROK, "Test function has been moved inside a thread?");

	TST_EXP_EQ_LI(tst_syscall(__NR_gettid), tst_syscall(__NR_getpid));
	TST_EXP_EQ_LI(tst_syscall(__NR_gettid), pid);
}

static struct tst_test test = {
	.test_all = run,
};
