/*
 * Copyright (c) 2016 Linux Test Project
 *
 * This program is free software;  you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY;  without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
 * the GNU General Public License for more details.
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
