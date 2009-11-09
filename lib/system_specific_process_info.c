/*
 *
 *   Copyright (c) International Business Machines  Corp., 2009
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
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

/*
 *   DESCRIPTION
 *	get_max_pids(): Return the maximum number of pids for this system by
 *			reading /proc/sys/kernel/pid_max
 *
 *	get_free_pids(): Return number of free pids by subtracting the number
 *			 of pids currently used ('ps -eT') from max_pids
 */


#include <fcntl.h>
#include <limits.h>
#include <sys/types.h>
#include "test.h"

#define BUFSIZE 512

int get_max_pids(void)
{
#ifdef __linux__

	FILE *f;
	char buf[BUFSIZE];

	f = fopen("/proc/sys/kernel/pid_max", "r");
	if (!f) {
		tst_resm(TBROK, "Could not open /proc/sys/kernel/pid_max");
		return -1;
	}
	if (!fgets(buf, BUFSIZE, f)) {
		fclose(f);
		tst_resm(TBROK, "Could not read /proc/sys/kernel/pid_max");
		return -1;
	}
	fclose(f);
	return atoi(buf);
#else
	return SHRT_MAX;
#endif
}


int get_free_pids(void)
{
	FILE *f;
	int rc, used_pids, max_pids;

	f = popen("ps -eT | wc -l", "r");
	if (!f) {
		tst_resm(TBROK, "Could not run 'ps' to calculate used "
				"pids");
		return -1;
	}
	rc = fscanf(f, "%i", &used_pids);
	pclose(f);

	if (rc != 1 || used_pids < 0) {
		tst_resm(TBROK, "Could not read output of 'ps' to "
				"calculate used pids");
		return -1;
	}

	max_pids = get_max_pids();

	if (max_pids < 0)
		return -1;

	return max_pids - used_pids;
}

