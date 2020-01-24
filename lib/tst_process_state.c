/*
 * Copyright (C) 2012-2014 Cyril Hrubis chrubis@suse.cz
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it would be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * Further, this software is distributed without any warranty that it is
 * free of the rightful claim of any third person regarding infringement
 * or the like.  Any license provided herein, whether implied or
 * otherwise, applies only to this software file.  Patent licenses, if
 * any, provided herein do not apply to combinations of this program with
 * other software, or any other product whatsoever.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
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
		                "%*i %*s %c", &cur_state);

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

		if (fscanf(f, "%*i %*s %c", &cur_state) != 1) {
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
