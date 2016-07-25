/*
 * Copyright (c) Huawei Technologies Co., Ltd., 2015
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See
 * the GNU General Public License for more details.
 */

/*
 * Verify that:
 * When a process with non-zero user IDs performs an execve(), the
 * process's capability sets are cleared. When a process with zero
 * user IDs performs an execve(), the process's capability sets
 * are set.
 */

#define _GNU_SOURCE
#include <sys/wait.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include "libclone.h"
#include "test.h"
#include "config.h"
#if HAVE_SYS_CAPABILITY_H
#include <sys/capability.h>
#endif

char *TCID = "userns06_capcheck";
int TST_TOTAL = 1;

int main(int argc, char *argv[])
{
#ifdef HAVE_LIBCAP
	cap_t caps;
	int i, last_cap;
	cap_flag_value_t flag_val;
	cap_flag_value_t expected_flag = 1;
#endif
	tst_parse_opts(argc, argv, NULL, NULL);

#ifdef HAVE_LIBCAP
	if (strcmp("privileged", argv[1]))
		expected_flag = 0;

	caps = cap_get_proc();
	SAFE_FILE_SCANF(NULL, "/proc/sys/kernel/cap_last_cap", "%d", &last_cap);
	for (i = 0; i <= last_cap; i++) {
		cap_get_flag(caps, i, CAP_EFFECTIVE, &flag_val);
		if (flag_val != expected_flag)
			break;
		cap_get_flag(caps, i, CAP_PERMITTED, &flag_val);
		if (flag_val != expected_flag)
			break;
	}

	if (flag_val != expected_flag) {
		printf("unexpected effective/permitted caps at %d\n", i);
		exit(1);
	}

#else
	printf("System is missing libcap.\n");
#endif
	tst_exit();
}
