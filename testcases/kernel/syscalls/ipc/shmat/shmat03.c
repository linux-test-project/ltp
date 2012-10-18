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
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

/*
 * NAME
 *	shmat03.c
 *
 * DESCRIPTION
 *	shmat03 - test for EACCES error
 *
 * ALGORITHM
 *	create a shared memory segment with root only read & write permissions
 *	fork a child process
 *	if child
 *	  set the ID of the child process to that of "nobody"
 *	  loop if that option was specified
 *	    call shmat() using the TEST() macro
 *	    check the errno value
 *	      issue a PASS message if we get EACCES
 *	    otherwise, the tests fails
 *	      issue a FAIL message
 *	  call cleanup
 *	if parent
 *	  wait for child to exit
 *	  remove the shared memory segment
 *
 * USAGE:  <for command-line>
 *  shmat03 [-c n] [-e] [-i n] [-I x] [-P x] [-t]
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
 *	test must be run at root
 */

#include "ipcshm.h"

char *TCID = "shmat03";
int TST_TOTAL = 1;

int exp_enos[] = { EACCES, 0 };	/* 0 terminated list of expected errnos */

int shm_id_1 = -1;

void *addr;			/* for result of shmat-call */

uid_t ltp_uid;
char *ltp_user = "nobody";

void do_child(void);

int main(int ac, char **av)
{
	char *msg;		/* message returned from parse_opts */
	int pid;

	/* parse standard options */
	if ((msg = parse_opts(ac, av, NULL, NULL)) != NULL) {
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);
	}

	setup();		/* global setup */

	if ((pid = FORK_OR_VFORK()) == -1) {
		tst_brkm(TBROK, cleanup, "could not fork");
	}

	if (pid == 0) {		/* child */
		/* set the user ID of the child to the non root user */
		if (setuid(ltp_uid) == -1) {
			perror("setuid() failed");
			exit(1);
		}

		do_child();

	} else {		/* parent */
		/* wait for the child to return */
		if (waitpid(pid, NULL, 0) == -1) {
			tst_brkm(TBROK, cleanup, "waitpid failed");
		}

		/* if it exists, remove the shared memory resource */
		rm_shm(shm_id_1);

		tst_rmdir();
	}

	cleanup();
	tst_exit();
}

/*
 * do_child - make the TEST call as the child process
 */
void do_child()
{
	int lc;

	/* The following loop checks looping state if -i option given */

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		/* reset Tst_count in case we are looping */
		Tst_count = 0;

		/*
		 * use TEST macro to make the call
		 */
		errno = 0;
		addr = shmat(shm_id_1, (const void *)0, 0);
		TEST_ERRNO = errno;

		if (addr != (char *)-1) {
			tst_resm(TFAIL, "call succeeded unexpectedly");
			continue;
		}

		TEST_ERROR_LOG(TEST_ERRNO);

		switch (TEST_ERRNO) {
		case EACCES:
			tst_resm(TPASS|TTERRNO, "expected failure");
			break;
		default:
			tst_resm(TFAIL|TTERRNO,
			    "call failed with an unexpected error");
			break;
		}
	}
}

/*
 * setup() - performs all the ONE TIME setup for this test.
 */
void setup(void)
{
	/* check for root as process owner */
	check_root();

	tst_sig(FORK, DEF_HANDLER, cleanup);

	/* Set up the expected error numbers for -e option */
	TEST_EXP_ENOS(exp_enos);

	TEST_PAUSE;

	/*
	 * Create a temporary directory and cd into it.
	 * This helps to ensure that a unique msgkey is created.
	 * See ../lib/libipc.c for more information.
	 */
	tst_tmpdir();

	/* get an IPC resource key */
	shmkey = getipckey();

	/* create a shared memory segment with read and write permissions */
	if ((shm_id_1 = shmget(shmkey, SHM_SIZE,
			       SHM_RW | IPC_CREAT | IPC_EXCL)) == -1) {
		tst_brkm(TBROK, cleanup, "Failed to create shared memory "
			 "segment in setup");
	}

	/* get the userid for a non root user */
	ltp_uid = getuserid(ltp_user);
}

/*
 * cleanup() - performs all the ONE TIME cleanup for this test at completion
 * 	       or premature exit.
 */
void cleanup(void)
{
	/*
	 * print timing stats if that option was specified.
	 * print errno log if that option was specified.
	 */
	TEST_CLEANUP;

}
