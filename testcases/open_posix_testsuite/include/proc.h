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

#ifdef __linux__
# include <errno.h>
# include <string.h>
int tst_process_state_wait3(pid_t pid, const char state,
	unsigned long maxwait_ms)
{
	char proc_path[128], cur_state;
	int wait_step_ms = 10;
	unsigned long waited_ms = 0;

	snprintf(proc_path, sizeof(proc_path), "/proc/%i/stat", pid);

	for (;;) {
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

		usleep(wait_step_ms * 1000);
		waited_ms += wait_step_ms;

		if (waited_ms >= maxwait_ms) {
			fprintf(stderr, "Reached max wait time\n");
			return 1;
		}
	}
}
#else
int tst_process_state_wait3(pid_t pid, const char state,
	unsigned long maxwait_ms)
{
	usleep(maxwait_ms * 1000);
	return 0;
}
#endif
