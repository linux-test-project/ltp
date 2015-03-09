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
 *	shmdt01.c
 *
 * DESCRIPTION
 *	shmdt01 - check that shared memory is detached correctly
 *
 * ALGORITHM
 *	create a shared memory resource of size sizeof(int)
 *	attach it to the current process and give it a value
 *	call shmdt() using the TEST macro
 *	check the return code
 *	  if failure, issue a FAIL message.
 *	otherwise,
 *	  if doing functionality testing
 *		attempt to write a value to the shared memory address
 *		this should generate a SIGSEGV which will be caught in
 *		    the signal handler
 *	  	if correct,
 *			issue a PASS message
 *		otherwise
 *			issue a FAIL message
 *	call cleanup
 *
 * USAGE:  <for command-line>
 *  shmdt01 [-c n] [-f] [-i n] [-I x] [-P x] [-t]
 *     where,  -c n : Run n copies concurrently.
 *             -f   : Turn off functionality Testing.
 *	       -I x : Execute test for x seconds.
 *	       -P x : Pause for x seconds between iterations.
 *	       -t   : Turn on syscall timing.
 *
 * HISTORY
 *	03/2001 - Written by Wayne Boyer
 *
 *      06/03/2008 Renaud Lottiaux (Renaud.Lottiaux@kerlabs.com)
 *      - Fix wrong return value check on shmat system call (leading to
 *        segfault in case of error with this syscall).
 *
 * RESTRICTIONS
 *	none
 */

#include <setjmp.h>
#include "ipcshm.h"

char *TCID = "shmdt01";
int TST_TOTAL = 1;

void sighandler(int);
struct shmid_ds buf;

int shm_id_1 = -1;
int *shared;			/* variable to use for shared memory attach */
int new;
int pass = 0;
sigjmp_buf env;

int main(int ac, char **av)
{
	int lc;
	void check_functionality(void);

	tst_parse_opts(ac, av, NULL, NULL);

	setup();		/* global setup */

	/* The following loop checks looping state if -i option given */

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		/* reset tst_count in case we are looping */
		tst_count = 0;

		/*
		 * Use TEST macro to make the shmdt() call
		 */

		TEST(shmdt((const void *)shared));

		if (TEST_RETURN == -1) {
			tst_resm(TFAIL, "%s call failed - errno = %d : %s",
				 TCID, TEST_ERRNO, strerror(TEST_ERRNO));
		} else {
			check_functionality();
		}

		/* reattach the shared memory segment in case we are looping */
		shared = shmat(shm_id_1, 0, 0);

		if (shared == (void *)-1) {
			tst_brkm(TBROK, cleanup, "memory reattach failed");
		}

		/* also reset pass */
		pass = 0;
	}

	cleanup();

	tst_exit();
}

/*
 * check_functionality() - make sure the memory is detached correctly
 */
void check_functionality(void)
{
	/* stat the shared memory segment */
	if (shmctl(shm_id_1, IPC_STAT, &buf) == -1)
		tst_brkm(TBROK | TERRNO, cleanup,
			 "could not stat in signal handler");

	if (buf.shm_nattch != 0) {
		tst_resm(TFAIL, "# of attaches is incorrect");
		return;
	}

	/*
	 * Try writing to the shared memory.  This should generate a
	 * SIGSEGV which will be caught below.
	 *
	 * This is wrapped by the sigsetjmp() call that will take care of
	 * restoring the program's context in an elegant way in conjunction
	 * with the call to siglongjmp() in the signal handler.
	 *
	 * An attempt to do the assignment without using the sigsetjmp()
	 * and siglongjmp() calls will result in an infinite loop.  Program
	 * control is returned to the assignment statement after the execution
	 * of the signal handler and another SIGSEGV will be generated.
	 */

	if (sigsetjmp(env, 1) == 0) {
		*shared = 2;
	}

	if (pass) {
		tst_resm(TPASS, "shared memory detached correctly");
	} else {
		tst_resm(TFAIL, "shared memory was not detached correctly");
	}
}

void sighandler(int sig)
{
	/* if we have received a SIGSEGV, we are almost done */
	if (sig == SIGSEGV) {
		/* set the global variable and jump back */
		pass = 1;
		siglongjmp(env, 1);
	} else
		tst_brkm(TBROK, cleanup,
			 "received an unexpected signal: %d", sig);
}

/*
 * setup() - performs all the ONE TIME setup for this test.
 */
void setup(void)
{

	tst_sig(NOFORK, sighandler, cleanup);

	TEST_PAUSE;

	/*
	 * Create a temporary directory and cd into it.
	 * This helps to ensure that a unique msgkey is created.
	 * See ../lib/libipc.c for more information.
	 */
	tst_tmpdir();

	/* get an IPC resource key */
	shmkey = getipckey();

	/* create a shared memory resource with read and write permissions */
	if ((shm_id_1 = shmget(shmkey, INT_SIZE, SHM_RW | IPC_CREAT |
			       IPC_EXCL)) == -1) {
		tst_brkm(TBROK, cleanup, "Failed to create shared memory "
			 "resource in setup()");
	}

	/* attach the shared memory segment */
	shared = shmat(shm_id_1, 0, 0);

	if (shared == (void *)-1) {
		tst_brkm(TBROK, cleanup, "Couldn't attach shared memory");
	}

	/* give a value to the shared memory integer */
	*shared = 4;
}

/*
 * cleanup() - performs all the ONE TIME cleanup for this test at completion
 * 	       or premature exit.
 */
void cleanup(void)
{
	/* if it exists, delete the shared memory resource */
	rm_shm(shm_id_1);

	tst_rmdir();

}
