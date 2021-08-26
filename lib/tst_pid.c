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

#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include "test.h"
#include "tst_pid.h"
#include "old_safe_file_ops.h"

#define PID_MAX_PATH "/proc/sys/kernel/pid_max"
#define CGROUPS_V1_SLICE_FMT "/sys/fs/cgroup/pids/user.slice/user-%d.slice/pids.max"
#define CGROUPS_V2_SLICE_FMT "/sys/fs/cgroup/user.slice/user-%d.slice/pids.max"
/* Leave some available processes for the OS */
#define PIDS_RESERVE 50

pid_t tst_get_unused_pid_(void (*cleanup_fn) (void))
{
	pid_t pid;

	SAFE_FILE_SCANF(cleanup_fn, PID_MAX_PATH, "%d", &pid);

	return pid;
}

/*
 * Get the effective session UID - either one invoking current test via sudo
 * or the real UID.
 */
static unsigned int get_session_uid(void)
{
	const char *sudo_uid;

	sudo_uid = getenv("SUDO_UID");
	if (sudo_uid) {
		unsigned int real_uid;
		int ret;

		ret = sscanf(sudo_uid, "%u", &real_uid);
		if (ret == 1)
			return real_uid;
	}

	return getuid();
}

static int read_session_pids_limit(const char *path_fmt, int uid,
				   void (*cleanup_fn) (void))
{
	int max_pids, ret;
	char path[PATH_MAX];

	ret = snprintf(path, sizeof(path), path_fmt, uid);
	if (ret < 0 || (size_t)ret >= sizeof(path))
		return -1;

	if (access(path, R_OK) != 0) {
		tst_resm(TINFO, "Cannot read session user limits from '%s'", path);
		return -1;
	}

	SAFE_FILE_SCANF(cleanup_fn, path, "%d", &max_pids);
	tst_resm(TINFO, "Found limit of processes %d (from %s)", max_pids, path);

	return max_pids;
}

static int get_session_pids_limit(void (*cleanup_fn) (void))
{
	int max_pids, uid;

	uid = get_session_uid();
	max_pids = read_session_pids_limit(CGROUPS_V2_SLICE_FMT, uid, cleanup_fn);
	if (max_pids < 0)
		max_pids = read_session_pids_limit(CGROUPS_V1_SLICE_FMT, uid,
						   cleanup_fn);

	if (max_pids < 0)
		return -1;

	return max_pids;
}

int tst_get_free_pids_(void (*cleanup_fn) (void))
{
	FILE *f;
	int rc, used_pids, max_pids, max_session_pids;

	f = popen("ps -eT | wc -l", "r");
	if (!f) {
		tst_brkm(TBROK, cleanup_fn, "Could not run 'ps' to calculate used pids");
		return -1;
	}
	rc = fscanf(f, "%i", &used_pids);
	pclose(f);

	if (rc != 1 || used_pids < 0) {
		tst_brkm(TBROK, cleanup_fn, "Could not read output of 'ps' to calculate used pids");
		return -1;
	}

	SAFE_FILE_SCANF(cleanup_fn, PID_MAX_PATH, "%d", &max_pids);

	max_session_pids = get_session_pids_limit(cleanup_fn);
	if ((max_session_pids > 0) && (max_session_pids < max_pids))
		max_pids = max_session_pids;

	if (max_pids > PIDS_RESERVE)
		max_pids -= PIDS_RESERVE;
	else
		max_pids = 0;

	/* max_pids contains the maximum PID + 1,
	 * used_pids contains used PIDs + 1,
	 * so this additional '1' is eliminated by the substraction */
	if (used_pids >= max_pids) {
		tst_brkm(TBROK, cleanup_fn, "No free pids");
		return 0;
	}
	return max_pids - used_pids;
}
