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
 *	semop04.c
 *
 * DESCRIPTION
 *	semop04 - test for EAGAIN error
 *
 * ALGORITHM
 *	create a semaphore set with read and alter permissions
 *	loop if that option was specified
 *	call semop() with two different invalid cases
 *	check the errno value
 *	  issue a PASS message if we get EAGAIN
 *	otherwise, the tests fails
 *	  issue a FAIL message
 *	call cleanup
 *
 * USAGE:  <for command-line>
 *  semop04 [-c n] [-e] [-i n] [-I x] [-P x] [-t]
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

char *TCID = "semop04";
int TST_TOTAL = 2;

int sem_id_1 = -1;

struct sembuf s_buf;

struct test_case_t {
	union semun get_arr;
	short op;
	short flg;
	short num;
	int error;
} TC[] = {
	/* EAGAIN sem_op = 0 */
	{ {
	1}, 0, IPC_NOWAIT, 2, EAGAIN},
	    /* EAGAIN sem_op = -1 */
	{ {
	0}, -1, IPC_NOWAIT, 2, EAGAIN}
};

int main(int ac, char **av)
{
	int lc;
	int val;		/* value for SETVAL */

	int i;

	tst_parse_opts(ac, av, NULL, NULL);

	setup();		/* global setup */

	/* The following loop checks looping state if -i option given */

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		/* reset tst_count in case we are looping */
		tst_count = 0;

		val = 1;
		for (i = 0; i < TST_TOTAL; i++) {

			/* initialize the s_buf buffer */
			s_buf.sem_op = TC[i].op;
			s_buf.sem_flg = TC[i].flg;
			s_buf.sem_num = TC[i].num;

			/* initialize all the primitive semaphores */
			TC[i].get_arr.val = val--;
			if (semctl(sem_id_1, TC[i].num, SETVAL, TC[i].get_arr)
			    == -1) {
				tst_brkm(TBROK, cleanup, "semctl() failed");
			}

			/*
			 * make the call with the TEST macro
			 */

			TEST(semop(sem_id_1, &s_buf, 1));

			if (TEST_RETURN != -1) {
				tst_resm(TFAIL, "call succeeded unexpectedly");
				continue;
			}

			if (TEST_ERRNO == TC[i].error) {
				tst_resm(TPASS,
					 "expected failure - errno = %d"
					 " : %s", TEST_ERRNO,
					 strerror(TEST_ERRNO));
			} else {
				tst_resm(TFAIL, "unexpected error - "
					 "%d : %s", TEST_ERRNO,
					 strerror(TEST_ERRNO));
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

	/* create a semaphore set with read and alter permissions */
	/* and PSEMS "primitive" semaphores                       */
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
