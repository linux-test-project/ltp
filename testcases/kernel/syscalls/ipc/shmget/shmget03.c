/*
 *
 *   Copyright (c) International Business Machines  Corp., 2001
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
 *	shmget03.c
 *
 * DESCRIPTION
 *	shmget03 - test for ENOSPC error
 *
 * ALGORITHM
 *	create shared memory segments in a loop until reaching the system limit
 *	loop if that option was specified
 *	  attempt to create yet another shared memory segment
 *	  check the errno value
 *	    issue a PASS message if we get ENOSPC
 *	  otherwise, the tests fails
 *	    issue a FAIL message
 *	call cleanup
 *
 * USAGE:  <for command-line>
 *  shmget03 [-c n] [-e] [-i n] [-I x] [-P x] [-t]
 *     where,  -c n : Run n copies concurrently.
 *             -e   : Turn on errno logging.
 *	       -i n : Execute test n times.
 *	       -I x : Execute test for x seconds.
 *	       -P x : Pause for x seconds between iterations.
 *	       -t   : Turn on syscall timing.
 *
 * HISTORY
 *	03/2001 - Written by Wayne Boyer
 *
 * RESTRICTIONS
 *	none
 */

#include "ipcshm.h"

char *TCID = "shmget03";
int TST_TOTAL = 1;

/*
 * The MAXIDS value is somewhat arbitrary and may need to be increased
 * depending on the system being tested.
 */
#define MAXIDS	8192

int shm_id_1 = -1;
int num_shms = 0;

int shm_id_arr[MAXIDS];

int main(int ac, char **av)
{
	int lc;

	tst_parse_opts(ac, av, NULL, NULL);

	setup();		/* global setup */

	/* The following loop checks looping state if -i option given */

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		/* reset tst_count in case we are looping */
		tst_count = 0;

		/*
		 * use the TEST() macro to make the call
		 */

		TEST(shmget(IPC_PRIVATE, SHM_SIZE, IPC_CREAT | IPC_EXCL
			    | SHM_RW));

		if (TEST_RETURN != -1) {
			tst_resm(TFAIL, "call succeeded when error expected");
			continue;
		}

		switch (TEST_ERRNO) {
		case ENOSPC:
			tst_resm(TPASS, "expected failure - errno = "
				 "%d : %s", TEST_ERRNO, strerror(TEST_ERRNO));
			break;
		default:
			tst_resm(TFAIL, "call failed with an "
				 "unexpected error - %d : %s",
				 TEST_ERRNO, strerror(TEST_ERRNO));
			break;
		}
	}

	cleanup();

	tst_exit();
}

/*
 * setup() - performs all the ONE TIME setup for this test.
 */
void setup(void)
{

	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;

	/*
	 * Create a temporary directory and cd into it.
	 * This helps to ensure that a unique msgkey is created.
	 * See ../lib/libipc.c for more information.
	 */
	tst_tmpdir();

	/* get an IPC resource key */
	shmkey = getipckey();

	/*
	 * Use a while loop to create the maximum number of memory segments.
	 * If the loop exceeds MAXIDS, then break the test and cleanup.
	 */
	while ((shm_id_1 = shmget(IPC_PRIVATE, SHM_SIZE, IPC_CREAT |
				  IPC_EXCL | SHM_RW)) != -1) {
		shm_id_arr[num_shms++] = shm_id_1;
		if (num_shms == MAXIDS) {
			tst_brkm(TBROK, cleanup, "The maximum number of shared "
				 "memory ID's has been\n\t reached.  Please "
				 "increase the MAXIDS value in the test.");
		}
	}

	/*
	 * If the errno is other than ENOSPC, then something else is wrong.
	 */
	if (errno != ENOSPC) {
		tst_resm(TINFO, "errno = %d : %s", errno, strerror(errno));
		tst_brkm(TBROK, cleanup, "Didn't get ENOSPC in test setup");
	}
}

/*
 * cleanup() - performs all the ONE TIME cleanup for this test at completion
 * 	       or premature exit.
 */
void cleanup(void)
{
	int i;

	/* remove the shared memory resources that were created */
	for (i = 0; i < num_shms; i++) {
		rm_shm(shm_id_arr[i]);
	}

	tst_rmdir();

}
