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
 *	shmget02.c
 *
 * DESCRIPTION
 *	shmget02 - check for ENOENT, EEXIST and EINVAL errors
 *
 * ALGORITHM
 *	create a shared memory segment with read & write permissions
 *	loop if that option was specified
 *	  call shmget() using five different invalid cases
 *	  check the errno value
 *	    issue a PASS message if we get ENOENT, EEXIST or EINVAL
 *	  otherwise, the tests fails
 *	    issue a FAIL message
 *	call cleanup
 *
 * USAGE:  <for command-line>
 *  shmget02 [-c n] [-e] [-i n] [-I x] [-P x] [-t]
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
 *      06/03/2008 Renaud Lottiaux (Renaud.Lottiaux@kerlabs.com)
 *      - Fix concurrency issue. The second key used for this test could
 *        conflict with the key from another task.
 *
 * RESTRICTIONS
 *	none
 */

#include "ipcshm.h"

char *TCID = "shmget02";
int TST_TOTAL = 4;

int shm_id_1 = -1;
int shm_nonexisting_key = -1;
key_t shmkey2;

struct test_case_t {
	int *skey;
	int size;
	int flags;
	int error;
} TC[] = {
	/* EINVAL - size is 0 */
	{
	&shmkey2, 0, IPC_CREAT | IPC_EXCL | SHM_RW, EINVAL},
	    /* EINVAL - size is negative */
//      {&shmkey2, -1, IPC_CREAT | IPC_EXCL | SHM_RW, EINVAL},
	    /* EINVAL - size is larger than created segment */
	{
	&shmkey, SHM_SIZE * 2, SHM_RW, EINVAL},
	    /* EEXIST - the segment exists and IPC_CREAT | IPC_EXCL is given */
	{
	&shmkey, SHM_SIZE, IPC_CREAT | IPC_EXCL | SHM_RW, EEXIST},
	    /* ENOENT - no segment exists for the key and IPC_CREAT is not given */
	    /* use shm_id_2 (-1) as the key */
	{
	&shm_nonexisting_key, SHM_SIZE, SHM_RW, ENOENT}
};

int main(int ac, char **av)
{
	int lc;
	int i;

	tst_parse_opts(ac, av, NULL, NULL);

	setup();		/* global setup */

	/* The following loop checks looping state if -i option given */

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		/* reset tst_count in case we are looping */
		tst_count = 0;

		/* loop through the test cases */
		for (i = 0; i < TST_TOTAL; i++) {
			/*
			 * Look for a failure ...
			 */

			TEST(shmget(*(TC[i].skey), TC[i].size, TC[i].flags));

			if (TEST_RETURN != -1) {
				tst_resm(TFAIL, "call succeeded unexpectedly");
				continue;
			}

			if (TEST_ERRNO == TC[i].error) {
				tst_resm(TPASS, "expected failure - errno = "
					 "%d : %s", TEST_ERRNO,
					 strerror(TEST_ERRNO));
			} else {
				tst_resm(TFAIL, "call failed with an "
					 "unexpected error - %d : %s",
					 TEST_ERRNO, strerror(TEST_ERRNO));
			}
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

	/* Get an new IPC resource key. */
	shmkey2 = getipckey();

	if ((shm_id_1 = shmget(shmkey, SHM_SIZE, IPC_CREAT | IPC_EXCL |
			       SHM_RW)) == -1) {
		tst_brkm(TBROK, cleanup, "couldn't create shared memory "
			 "segment in setup()");
	}

	/* Make sure shm_nonexisting_key is a nonexisting key */
	while (1) {
		while (-1 != shmget(shm_nonexisting_key, 1, SHM_RD)) {
			shm_nonexisting_key--;
		}
		if (errno == ENOENT)
			break;
	}
}

/*
 * cleanup() - performs all the ONE TIME cleanup for this test at completion
 * 	       or premature exit.
 */
void cleanup(void)
{
	/* if it exists, remove the shared memory resource */
	rm_shm(shm_id_1);

	tst_rmdir();

}
