/*
 *
 *   Copyright (c) International Business Machines  Corp., 2009
 *   Copyright (c) 2014 Oracle and/or its affiliates. All Rights Reserved.
 *
 *   This program is free software;  you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY;  without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
 *   the GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program;  if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include <fcntl.h>
#include <limits.h>
#include <sys/types.h>
#include "test.h"
#include "tst_pid.h"
#include "old_safe_file_ops.h"

#define PID_MAX_PATH "/proc/sys/kernel/pid_max"

pid_t tst_get_unused_pid_(void (*cleanup_fn) (void))
{
	pid_t pid;

	SAFE_FILE_SCANF(cleanup_fn, PID_MAX_PATH, "%d", &pid);

	return pid;
}

int tst_get_free_pids_(void (*cleanup_fn) (void))
{
	FILE *f;
	int rc, used_pids, max_pids;

	f = popen("ps -eT | wc -l", "r");
	if (!f) {
		tst_resm(TBROK, "Could not run 'ps' to calculate used " "pids");
		return -1;
	}
	rc = fscanf(f, "%i", &used_pids);
	pclose(f);

	if (rc != 1 || used_pids < 0) {
		tst_resm(TBROK, "Could not read output of 'ps' to "
			 "calculate used pids");
		return -1;
	}

	SAFE_FILE_SCANF(cleanup_fn, PID_MAX_PATH, "%d", &max_pids);

	/* max_pids contains the maximum PID + 1,
	 * used_pids contains used PIDs + 1,
	 * so this additional '1' is eliminated by the substraction */
	return max_pids - used_pids;
}
