// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) Huawei Technologies Co., Ltd., 2015
 * Copyright (C) 2022 SUSE LLC Andrea Cervesato <andrea.cervesato@suse.com>
 *
 * When a process with non-zero user IDs performs an execve(), the
 * process's capability sets are cleared. When a process with zero
 * user IDs performs an execve(), the process's capability sets
 * are set.
 */

#define TST_NO_DEFAULT_MAIN
#include "tst_test.h"
#include "config.h"

#ifdef HAVE_LIBCAP
#define _GNU_SOURCE

#include <string.h>
#include <sys/wait.h>
#include <sys/capability.h>

int main(int argc, char *argv[])
{
	cap_t caps;
	int i, last_cap;
	cap_flag_value_t cap_flag;
	cap_flag_value_t expected_cap_flag = 1;

	tst_reinit();

	if (argc < 2)
		tst_brk(TBROK, "userns06_capcheck <privileged|unprivileged>");

	SAFE_FILE_SCANF("/proc/sys/kernel/cap_last_cap", "%d", &last_cap);

	if (strcmp("privileged", argv[1]))
		expected_cap_flag = 0;

	caps = cap_get_proc();

	for (i = 0; i <= last_cap; i++) {
		cap_get_flag(caps, i, CAP_EFFECTIVE, &cap_flag);
		if (cap_flag != expected_cap_flag)
			break;

		cap_get_flag(caps, i, CAP_PERMITTED, &cap_flag);
		if (cap_flag != expected_cap_flag)
			break;
	}

	TST_EXP_EQ_LI(cap_flag, expected_cap_flag);

	return 0;
}

#else
int main(void)
{
	tst_reinit();

	tst_brk(TCONF, "System is missing libcap");

	return 0;
}
#endif
