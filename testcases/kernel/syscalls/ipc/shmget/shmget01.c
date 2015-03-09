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
 *	shmget01.c
 *
 * DESCRIPTION
 *	shmget01 - test that shmget() correctly creates a shared memory segment
 *
 * ALGORITHM
 *	loop if that option was specified
 *	use the TEST() macro to call shmget()
 *	check the return code
 *	  if failure, issue a FAIL message.
 *	otherwise,
 *	  if doing functionality testing
 *		stat the shared memory resource
 *		check the size, creator pid and mode
 *	  	if correct,
 *			issue a PASS message
 *		otherwise
 *			issue a FAIL message
 *	  else issue a PASS message
 *	call cleanup
 *
 * USAGE:  <for command-line>
 *  shmget01 [-c n] [-f] [-i n] [-I x] [-P x] [-t]
 *     where,  -c n : Run n copies concurrently.
 *             -f   : Turn off functionality Testing.
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

char *TCID = "shmget01";
int TST_TOTAL = 1;

int shm_id_1 = -1;

int main(int ac, char **av)
{
	int lc;
	struct shmid_ds buf;

	tst_parse_opts(ac, av, NULL, NULL);

	setup();		/* global setup */

	/* The following loop checks looping state if -i option given */

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		/* reset tst_count in case we are looping */
		tst_count = 0;

		/*
		 * Use TEST macro to make the call
		 */

		TEST(shmget(shmkey, SHM_SIZE, (IPC_CREAT | IPC_EXCL | SHM_RW)));

		if (TEST_RETURN == -1) {
			tst_resm(TFAIL, "%s call failed - errno = %d : %s",
				 TCID, TEST_ERRNO, strerror(TEST_ERRNO));
		} else {
			shm_id_1 = TEST_RETURN;
			/* do a STAT and check some info */
			if (shmctl(shm_id_1, IPC_STAT, &buf) == -1) {
				tst_resm(TBROK, "shmctl failed in "
					 "functional test");
				continue;
			}
			/* check the seqment size */
			if (buf.shm_segsz != SHM_SIZE) {
				tst_resm(TFAIL, "seqment size is not "
					 "correct");
				continue;
			}
			/* check the pid of the creator */
			if (buf.shm_cpid != getpid()) {
				tst_resm(TFAIL, "creator pid is not "
					 "correct");
				continue;
			}
			/*
			 * check the mode of the seqment
			 * mask out all but the lower 9 bits
			 */
			if ((buf.shm_perm.mode & MODE_MASK) !=
			    ((SHM_RW) & MODE_MASK)) {
				tst_resm(TFAIL, "segment mode is not "
					 "correct");
				continue;
			}
			/* if we get here, everything looks good */
			tst_resm(TPASS, "size, pid & mode are correct");
		}

		/*
		 * clean up things in case we are looping
		 */
		if (shmctl(shm_id_1, IPC_RMID, NULL) == -1) {
			tst_resm(TBROK, "couldn't remove shared memory");
		} else {
			shm_id_1 = -1;
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
