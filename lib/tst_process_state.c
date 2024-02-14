// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2012-2014 Cyril Hrubis chrubis@suse.cz
 * Copyright (c) 2021 Xie Ziyao <xieziyao@huawei.com>
 */

#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "test.h"
#include "tst_process_state.h"

int tst_process_state_wait(const char *file, const int lineno,
			   void (*cleanup_fn)(void), pid_t pid,
			   const char state, unsigned int msec_timeout)
{
	char proc_path[128], cur_state;
	unsigned int msecs = 0;

	snprintf(proc_path, sizeof(proc_path), "/proc/%i/stat", pid);

	for (;;) {
		safe_file_scanf(file, lineno, cleanup_fn, proc_path,
				"%*[^)]%*c %c", &cur_state);

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

int tst_process_state_wait2(pid_t pid, const char state)
{
	char proc_path[128], cur_state;

	snprintf(proc_path, sizeof(proc_path), "/proc/%i/stat", pid);

	for (;;) {
		FILE *f = fopen(proc_path, "r");

		if (!f) {
			fprintf(stderr, "Failed to open '%s': %s\n",
				proc_path, strerror(errno));
			return 1;
		}

		if (fscanf(f, "%*[^)]%*c %c", &cur_state) != 1) {
			fclose(f);
			fprintf(stderr, "Failed to read '%s': %s\n",
				proc_path, strerror(errno));
			return 1;
		}
		fclose(f);

		if (state == cur_state)
			return 0;

		usleep(10000);
	}
}

int tst_process_exit_wait(pid_t pid, unsigned int msec_timeout)
{
	unsigned int msecs = 0;

	for (;;) {
		if (kill(pid, 0) && errno == ESRCH)
			break;

		usleep(1000);
		msecs += 1;

		if (msec_timeout && msecs >= msec_timeout) {
			errno = ETIMEDOUT;
			return 0;
		}
	}

	return 1;
}
