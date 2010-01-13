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
 *	mkdir04
 *
 * DESCRIPTION
 *
 * ALGORITHM
 *	Setup:
 *		Setup signal handling.
 *		Pause for SIGUSR1 if option specified.
 *		Create temporary directory.
 *
 *	Test:
 *		Loop if the proper options are given.
 *              fork the first child
 *                      set to be ltpuser1
 *                      create a dirctory tstdir1 with 0700 permission
 *              fork the second child
 *                      set to ltpuser2
 *                      try to create a subdirectory tstdir2 under tstdir1
 *                      check the returnvalue, if succeeded (return=0)
 *			       Log the errno and Issue a FAIL message.
 *		        Otherwise,
 *			       Verify the errno
 *			       if equals to EACCES,
 *				       Issue Pass message.
 *			       Otherwise,
 *				       Issue Fail message.
 *	Cleanup:
 *		Print errno log and/or timing stats if options given
 *		Delete the temporary directory created.
 * USAGE
 *	mkdir04 [-c n] [-e] [-f] [-i n] [-I x] [-P x] [-t]
 *	where,  -c n : Run n copies concurrently.
 *		-e   : Turn on errno logging.
 *		-f   : Turn off functionality Testing.
 *		-i n : Execute test n times.
 *		-I x : Execute test for x seconds.
 *		-P x : Pause for x seconds between iterations.
 *		-t   : Turn on syscall timing.
 *
 * HISTORY
 *	07/2001 Ported by Wayne Boyer
 *
 * RESTRICTIONS
 *	None.
 *
 */

#include <errno.h>
#include <string.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <pwd.h>
#include <sys/wait.h>
#include <unistd.h>
#include "test.h"
#include "usctest.h"

void setup();
void cleanup();
extern struct passwd *my_getpwnam(char *);
int fail;

#define PERMS		0700

char user1name[] = "nobody";
char user2name[] = "bin";

char *TCID = "mkdir04";		/* Test program identifier.    */
int TST_TOTAL = 1;		/* Total number of test cases. */
extern int Tst_count;		/* Test Case counter for tst_* routines */
int fail;

char tstdir1[100];
char tstdir2[100];

int exp_enos[] = { EACCES, 0 };	/* List must end with 0 */

int main(int ac, char **av)
{
	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */
	int rval;
	pid_t pid, pid1;
	int status;
	struct passwd *ltpuser1, *ltpuser2;

	/*
	 * parse standard options
	 */
	if ((msg = parse_opts(ac, av, (option_t *) NULL, NULL)) != (char *)NULL) {
		tst_brkm(TBROK, cleanup, "OPTION PARSING ERROR - %s", msg);
	}

	/*
	 * perform global setup for test
	 */
	setup();

	/* set the expected errnos... */
	TEST_EXP_ENOS(exp_enos);

	/*
	 * check looping state if -i option given
	 */
	for (lc = 0; TEST_LOOPING(lc); lc++) {

		/* reset Tst_count in case we are looping. */
		Tst_count = 0;

		/* Initialize the test directories name */
		sprintf(tstdir1, "tstdir1.%d", getpid());
		ltpuser1 = my_getpwnam(user1name);

		if ((pid = FORK_OR_VFORK()) < 0) {
			tst_brkm(TBROK, cleanup, "fork #1 failed");
		 /*NOTREACHED*/}

		if (pid == 0) {	/* first child */
			/* set to ltpuser1 */
			rval = setreuid(ltpuser1->pw_uid, ltpuser1->pw_uid);
			if (rval < 0) {
				tst_resm(TFAIL, "setreuid failed to "
					 "to set the real uid to %d and "
					 "effective uid to %d",
					 ltpuser1->pw_uid, ltpuser1->pw_uid);
				perror("setreuid");
				exit(1);
			 /*NOTREACHED*/}
			/* create the parent directory with 0700 permits */
			if (mkdir(tstdir1, PERMS) == -1) {
				tst_resm(TFAIL, "mkdir(%s, %#o) Failed",
					 tstdir1, PERMS);
				exit(1);
			 /*NOTREACHED*/}
			/* create tstdir1 succeeded */
			exit(0);
		}
		wait(&status);
		if (WEXITSTATUS(status) != 0) {
			tst_brkm(TFAIL, cleanup,
				 "Test to check mkdir EACCES failed"
				 "in create parent directory");
		}

		sprintf(tstdir2, "%s/tst", tstdir1);
		ltpuser2 = my_getpwnam(user2name);

		if ((pid1 = FORK_OR_VFORK()) < 0) {
			tst_brkm(TBROK, cleanup, "fork #2 failed");
		 /*NOTREACHED*/}

		if (pid1 == 0) {	/* second child */
			/* set to ltpuser2 */
			rval = setreuid(ltpuser2->pw_uid, ltpuser2->pw_uid);
			if (rval < 0) {
				tst_resm(TFAIL, "setreuid failed to "
					 "to set the real uid to %d and "
					 "effective uid to %d",
					 ltpuser2->pw_uid, ltpuser2->pw_uid);
				perror("setreuid");
				exit(1);
			 /*NOTREACHED*/}
			if (mkdir(tstdir2, PERMS) != -1) {
				tst_resm(TFAIL, "mkdir(%s, %#o) unexpected "
					 "succeeded", tstdir2, PERMS);
				exit(1);
			 /*NOTREACHED*/}
			if (errno != EACCES) {
				tst_resm(TFAIL, "Expected EACCES got %d",
					 errno);
				exit(1);
			 /*NOTREACHED*/}
			/* PASS */
			exit(0);
		}
		waitpid(pid1, &status, 0);
		if (WEXITSTATUS(status) == 0) {
			tst_resm(TPASS, "Test to attempt to creat a directory "
				 "in a directory having no permissions "
				 "SUCCEEDED in setting errno to EACCES");
		} else {
			tst_resm(TFAIL, "Test to attempt to creat a directory "
				 "in a directory having no permissions FAILED");
			cleanup();
		}
	}			/* End for TEST_LOOPING */

	/*
	 * cleanup and exit
	 */
	cleanup();

	 /*NOTREACHED*/ return 0;
}				/* End main */

/*
 * setup() - performs all ONE TIME setup for this test.
 */
void setup()
{
	/* must run as root */
	if (geteuid() != 0) {
		tst_brkm(TBROK, tst_exit, "Must run this as root");
	 /*NOTREACHED*/}

	/* capture signals */
	tst_sig(FORK, DEF_HANDLER, cleanup);

	/* Pause if that option was specified */
	TEST_PAUSE;

	/* Create a temporary directory and make it current. */
	tst_tmpdir();

	umask(0);
}

/*
 * cleanup() - performs all ONE TIME cleanup for this test at
 *             completion or premature exit.
 */
void cleanup()
{
	/*
	 * print timing stats if that option was specified.
	 * print errno log if that option was specified.
	 */
	TEST_CLEANUP;

	/*
	 * Remove the temporary directory.
	 */
	tst_rmdir();

	/*
	 * Exit with return code appropriate for results.
	 */
	tst_exit();
}
