// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2016 Linux Test Project
 */

#include <stdlib.h>
#include <sys/types.h>

#include "tst_sig_proc.h"

#define TST_NO_DEFAULT_MAIN
#include "tst_test.h"

pid_t create_sig_proc(int sig, int count, unsigned int usec)
{
	pid_t pid, cpid;

	pid = getpid();
	cpid = SAFE_FORK();

	if (cpid == 0) {
		while (count-- > 0) {
			usleep(usec);
			if (kill(pid, sig) == -1)
				break;
		}
		exit(0);
	}

	return cpid;
}
