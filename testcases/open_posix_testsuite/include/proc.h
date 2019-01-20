/*
 * Copyright (c) 2017 Linux Test Project
 *
 * This program is free software;  you can redistribute it and/or modify
 * it under the terms in version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY;  without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
 * the GNU General Public License for more details.
 */

#include <time.h>

#include "posixtest.h"

#ifdef __linux__
# include <errno.h>
# include <string.h>

int tst_process_state_wait3(pid_t pid, const char state,
	long maxwait_s)
{
	char proc_path[128], cur_state;
	int wait_step_ms = 10;
	struct timespec wait_step_ts;
	long iteration, max_iterations = (maxwait_s * 1000) / wait_step_ms;

	wait_step_ts.tv_sec = 0;
	wait_step_ts.tv_nsec = wait_step_ms * 1000000;

	snprintf(proc_path, sizeof(proc_path), "/proc/%i/stat", pid);

	for (iteration = 0; iteration < max_iterations; iteration++) {
		FILE *f = fopen(proc_path, "r");

		if (!f) {
			fprintf(stderr, "Failed to open '%s': %s\n",
				proc_path, strerror(errno));
			return 1;
		}

		if (fscanf(f, "%*i %*s %c", &cur_state) != 1) {
			fclose(f);
			fprintf(stderr, "Failed to read '%s': %s\n",
				proc_path, strerror(errno));
			return 1;
		}
		fclose(f);

		if (state == cur_state)
			return 0;

		nanosleep(&wait_step_ts, NULL);
	}
	fprintf(stderr, "Reached max wait time\n");
	return 1;
}
#else
int tst_process_state_wait3(pid_t pid LTP_ATTRIBUTE_UNUSED,
	const char state LTP_ATTRIBUTE_UNUSED, long maxwait_s)
{
	struct timespec maxwait_ts;

	maxwait_ts.tv_sec = maxwait_s;
	maxwait_ts.tv_nsec = 0;

	nanosleep(&maxwait_ts, NULL);
	return 0;
}
#endif
