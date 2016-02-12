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
#include "safe_macros.h"

void setup();
void cleanup();
int fail;

#define PERMS		0700

static uid_t nobody_uid, bin_uid;

char *TCID = "mkdir04";
int TST_TOTAL = 1;
int fail;

char tstdir1[100];
char tstdir2[100];

int main(int ac, char **av)
{
	int lc;
	int rval;
	pid_t pid, pid1;
	int status;

	/*
	 * parse standard options
	 */
	tst_parse_opts(ac, av, NULL, NULL);

	/*
	 * perform global setup for test
	 */
	setup();

	/*
	 * check looping state if -i option given
	 */
	for (lc = 0; TEST_LOOPING(lc); lc++) {

		tst_count = 0;

		/* Initialize the test directories name */
		sprintf(tstdir1, "tstdir1.%d", getpid());
		if ((pid = FORK_OR_VFORK()) < 0) {
			tst_brkm(TBROK, cleanup, "fork #1 failed");
		}

		if (pid == 0) {	/* first child */
			rval = setreuid(nobody_uid, nobody_uid);
			if (rval < 0) {
				tst_resm(TFAIL | TERRNO, "setreuid failed to "
					 "to set the real uid to %d and "
					 "effective uid to %d",
					 nobody_uid, nobody_uid);
				exit(1);
			}
			/* create the parent directory with 0700 permits */
			if (mkdir(tstdir1, PERMS) == -1) {
				tst_resm(TFAIL | TERRNO,
					 "mkdir(%s, %#o) Failed",
					 tstdir1, PERMS);
				exit(1);
			}
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

		if ((pid1 = FORK_OR_VFORK()) < 0) {
			tst_brkm(TBROK, cleanup, "fork #2 failed");
		}

		if (pid1 == 0) {	/* second child */
			rval = setreuid(bin_uid, bin_uid);
			if (rval < 0) {
				tst_resm(TFAIL | TERRNO, "setreuid failed to "
					 "to set the real uid to %d and "
					 "effective uid to %d",
					 bin_uid, bin_uid);
				exit(1);
			}
			if (mkdir(tstdir2, PERMS) != -1) {
				tst_resm(TFAIL, "mkdir(%s, %#o) unexpected "
					 "succeeded", tstdir2, PERMS);
				exit(1);
			}
			if (errno != EACCES) {
				tst_resm(TFAIL, "Expected EACCES got %d",
					 errno);
				exit(1);
			}
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
	}

	/*
	 * cleanup and exit
	 */
	cleanup();
	tst_exit();

}

/*
 * setup() - performs all ONE TIME setup for this test.
 */
void setup(void)
{
	struct passwd *pw;

	tst_require_root();

	pw = SAFE_GETPWNAM(NULL, "nobody");
	nobody_uid = pw->pw_uid;
	pw = SAFE_GETPWNAM(NULL, "bin");
	bin_uid = pw->pw_uid;

	tst_sig(FORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;

	/* Create a temporary directory and make it current. */
	tst_tmpdir();

	umask(0);
}

/*
 * cleanup() - performs all ONE TIME cleanup for this test at
 *             completion or premature exit.
 */
void cleanup(void)
{

	/*
	 * Remove the temporary directory.
	 */
	tst_rmdir();

	/*
	 * Exit with return code appropriate for results.
	 */

}
