/*
 *
 *   Copyright (c) International Business Machines  Corp., 2004
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

/*
 * NAME
 *	hugeshmget03.c
 *
 * DESCRIPTION
 *	hugeshmget03 - test for ENOSPC error
 *
 * ALGORITHM
 *	create large shared memory segments in a loop until reaching
 *		the system limit
 *	loop if that option was specified
 *	  attempt to create yet another shared memory segment
 *	  check the errno value
 *	    issue a PASS message if we get ENOSPC
 *	  otherwise, the tests fails
 *	    issue a FAIL message
 *	call cleanup
 *
 * USAGE:  <for command-line>
 *  hugeshmget03 [-c n] [-e] [-i n] [-I x] [-P x] [-t]
 *     where,  -c n : Run n copies concurrently.
 *             -e   : Turn on errno logging.
 *	       -i n : Execute test n times.
 *	       -I x : Execute test for x seconds.
 *	       -P x : Pause for x seconds between iterations.
 *	       -t   : Turn on syscall timing.
 *
 * HISTORY
 *	03/2001 - Written by Wayne Boyer
 *	04/2004 - Updated by Robbie Williamson
 *
 * RESTRICTIONS
 *	none
 */

#include "hugetlb.h"
#include "safe_macros.h"
#include "mem.h"

char *TCID = "hugeshmget03";
int TST_TOTAL = 1;

/*
 * The MAXIDS value is somewhat arbitrary and may need to be increased
 * depending on the system being tested.
 */
#define MAXIDS	8192
#define PATH_SHMMNI	"/proc/sys/kernel/shmmni"

static size_t shm_size;
static int shm_id_1 = -1;
static int num_shms;
static int shm_id_arr[MAXIDS];

static long hugepages = 128;
static long orig_shmmni;
static option_t options[] = {
	{"s:", &sflag, &nr_opt},
	{NULL, NULL, NULL}
};

int main(int ac, char **av)
{
	int lc;

	tst_parse_opts(ac, av, options, NULL);

	if (sflag)
		hugepages = SAFE_STRTOL(NULL, nr_opt, 0, LONG_MAX);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		tst_count = 0;

		TEST(shmget(IPC_PRIVATE, shm_size,
			    SHM_HUGETLB | IPC_CREAT | IPC_EXCL | SHM_RW));
		if (TEST_RETURN != -1) {
			tst_resm(TFAIL, "shmget succeeded unexpectedly");
			continue;
		}
		if (TEST_ERRNO == ENOSPC)
			tst_resm(TPASS | TTERRNO, "shmget failed as expected");
		else
			tst_resm(TFAIL | TTERRNO, "shmget failed unexpectedly "
				 "- expect errno=ENOSPC, got");
	}
	cleanup();
	tst_exit();
}

void setup(void)
{
	long hpage_size;

	tst_require_root();
	check_hugepage();
	tst_sig(NOFORK, DEF_HANDLER, cleanup);
	tst_tmpdir();

	orig_hugepages = get_sys_tune("nr_hugepages");
	SAFE_FILE_SCANF(NULL, PATH_SHMMNI, "%ld", &orig_shmmni);

	set_sys_tune("nr_hugepages", hugepages, 1);
	SAFE_FILE_PRINTF(NULL, PATH_SHMMNI, "%ld", hugepages / 2);

	hpage_size = read_meminfo("Hugepagesize:") * 1024;
	shm_size = hpage_size;

	/*
	 * Use a while loop to create the maximum number of memory segments.
	 * If the loop exceeds MAXIDS, then break the test and cleanup.
	 */
	num_shms = 0;
	shm_id_1 = shmget(IPC_PRIVATE, shm_size,
			  SHM_HUGETLB | IPC_CREAT | IPC_EXCL | SHM_RW);
	while (shm_id_1 != -1) {
		shm_id_arr[num_shms++] = shm_id_1;
		if (num_shms == MAXIDS)
			tst_brkm(TBROK, cleanup, "The maximum number of "
				 "shared memory ID's has been reached. "
				 "Please increase the MAXIDS value in "
				 "the test.");
		shm_id_1 = shmget(IPC_PRIVATE, shm_size,
				  SHM_HUGETLB | IPC_CREAT | IPC_EXCL | SHM_RW);
	}
	if (errno != ENOSPC)
		tst_brkm(TBROK | TERRNO, cleanup, "shmget #setup");

	TEST_PAUSE;
}

void cleanup(void)
{
	int i;

	for (i = 0; i < num_shms; i++)
		rm_shm(shm_id_arr[i]);

	FILE_PRINTF(PATH_SHMMNI, "%ld", orig_shmmni);
	set_sys_tune("nr_hugepages", orig_hugepages, 0);

	tst_rmdir();
}
