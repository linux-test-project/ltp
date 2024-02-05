// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2022 SUSE LLC Andrea Cervesato <andrea.cervesato@suse.com>
 */

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include "tst_safe_file_ops.h"
#include "tst_process_state.h"

int tst_thread_state_wait(pid_t tid, const char state,
			  unsigned int msec_timeout)
{
	char proc_path[128], cur_state;
	unsigned int msecs = 0;

	snprintf(proc_path, sizeof(proc_path), "/proc/self/task/%i/stat", tid);

	for (;;) {
		SAFE_FILE_SCANF(proc_path, "%*[^)]%*c %c", &cur_state);

		if (state == cur_state)
			break;

		usleep(1000);
		msecs += 1;

		if (msec_timeout && msecs >= msec_timeout) {
			errno = ETIMEDOUT;
			return -1;
		}
	}

	return 0;
}
