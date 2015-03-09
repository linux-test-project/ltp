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
 *	semop03.c
 *
 * DESCRIPTION
 *	semop03 - test for EFBIG error
 *
 * ALGORITHM
 *	create a semaphore set with read and alter permissions
 *	loop if that option was specified
 *	call semop() using two different invalid cases
 *	check the errno value
 *	  issue a PASS message if we get EFBIG
 *	otherwise, the tests fails
 *	  issue a FAIL message
 *	call cleanup
 *
 * USAGE:  <for command-line>
 *  semop03 [-c n] [-e] [-i n] [-I x] [-P x] [-t]
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

#include "ipcsem.h"

char *TCID = "semop03";
int TST_TOTAL = 2;

int sem_id_1 = -1;

struct sembuf s_buf;

int TC[] = { -1, PSEMS + 1 };	/* negative and too many "primitive" semas */

int main(int ac, char **av)
{
	int lc;
	int i;

	tst_parse_opts(ac, av, NULL, NULL);

	setup();		/* global setup */

	/* initialize two fields in the sembuf structure here */
	s_buf.sem_op = 1;	/* add this value to struct sem.semval */
	s_buf.sem_flg = SEM_UNDO;	/* undo when process exits */

	/* The following loop checks looping state if -i option given */

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		/* reset tst_count in case we are looping */
		tst_count = 0;

		for (i = 0; i < TST_TOTAL; i++) {

			/* initialize the last field in the sembuf */
			/* structure to the test dependent value   */
			s_buf.sem_num = TC[i];

			/*
			 * use the TEST macro to make the call
			 */

			TEST(semop(sem_id_1, &s_buf, 1));

			if (TEST_RETURN != -1) {
				tst_resm(TFAIL, "call succeeded unexpectedly");
				continue;
			}

			switch (TEST_ERRNO) {
			case EFBIG:
				tst_resm(TPASS, "expected failure - errno = "
					 "%d : %s", TEST_ERRNO,
					 strerror(TEST_ERRNO));
				break;
			default:
				tst_resm(TFAIL, "unexpected error - "
					 "%d : %s", TEST_ERRNO,
					 strerror(TEST_ERRNO));
				break;
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
	semkey = getipckey();

	/* create a semaphore with read and alter permissions */
	if ((sem_id_1 =
	     semget(semkey, PSEMS, IPC_CREAT | IPC_EXCL | SEM_RA)) == -1) {
		tst_brkm(TBROK, cleanup, "couldn't create semaphore in setup");
	}
}

/*
 * cleanup() - performs all the ONE TIME cleanup for this test at completion
 * 	       or premature exit.
 */
void cleanup(void)
{
	/* if it exists, remove the semaphore resource */
	rm_sema(sem_id_1);

	tst_rmdir();

}
