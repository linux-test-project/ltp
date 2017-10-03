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
 *	rename09
 *
 * DESCRIPTION
 *      check rename() fails with EACCES
 *
 * ALGORITHM
 *	Setup:
 *		Setup signal handling.
 *		Create temporary directory.
 *		Pause for SIGUSR1 if option specified.
 *
 *	Test:
 *		Loop if the proper options are given.
 *              fork the first child
 *                      set to be nobody
 *                      create old dir with mode 0700
 *                      creat a file under it
 *              fork the second child
 *                      set to bin
 *                      create new dir with mode 0700
 *                      create a "new" file under it
 *                      try to rename file under old dir to file under new dir
 *                      check the return value, if succeeded (return=0)
 *			       Issue a FAIL message.
 *		        Otherwise,
 *			       Log the errno
 *			       Verify the errno
 *			       if equals to EACCESS,
 *				       Issue Pass message.
 *			       Otherwise,
 *				       Issue Fail message.
 *	Cleanup:
 *		Print errno log and/or timing stats if options given
 *		Delete the temporary directory created.
 *
 * USAGE
 *	rename09 [-c n] [-e] [-i n] [-I x] [-P x] [-t]
 *	where,  -c n : Run n copies concurrently.
 *		-e   : Turn on errno logging.
 *		-i n : Execute test n times.
 *		-I x : Execute test for x seconds.
 *		-P x : Pause for x seconds between iterations.
 *		-t   : Turn on syscall timing.
 *
 * HISTORY
 *	07/2001 Ported by Wayne Boyer
 *
 * RESTRICTIONS
 *	Must run test as root.
 *
 */
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pwd.h>
#include <sys/wait.h>
#include <unistd.h>

#include "test.h"
#include "safe_macros.h"

void setup();
void cleanup();

#define PERMS		0700

char *TCID = "rename09";
int TST_TOTAL = 1;

char fdir[255], mdir[255];
char fname[255], mname[255];
uid_t nobody_uid, bin_uid;

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

		if ((pid = FORK_OR_VFORK()) == -1) {
			tst_brkm(TBROK, cleanup, "fork() #1 failed");
		}

		if (pid == 0) {	/* first child */
			/* set to nobody */
			rval = setreuid(nobody_uid, nobody_uid);
			if (rval < 0) {
				tst_resm(TWARN, "setreuid failed to "
					 "to set the real uid to %d and "
					 "effective uid to %d",
					 nobody_uid, nobody_uid);
				perror("setreuid");
				exit(1);
			}

			/* create the a directory with 0700 permits */
			if (mkdir(fdir, PERMS) == -1) {
				tst_resm(TWARN, "mkdir(%s, %#o) Failed",
					 fdir, PERMS);
				exit(1);
			}

			/* create "old" file under it */
			SAFE_TOUCH(cleanup, fname, 0700, NULL);

			exit(0);
		}

		/* wait for child to exit */
		wait(&status);
		if (!WIFEXITED(status) || (WEXITSTATUS(status) != 0)) {
			tst_brkm(TBROK, cleanup, "First child failed to set "
				 "up conditions for the test");
		}

		if ((pid1 = FORK_OR_VFORK()) == -1) {
			tst_brkm(TBROK, cleanup, "fork() #2 failed");
		}

		if (pid1 == 0) {	/* second child */
			/* set to bin */
			if ((rval = seteuid(bin_uid)) == -1) {
				tst_resm(TWARN, "seteuid() failed");
				perror("setreuid");
				exit(1);
			}

			/* create "new" directory */
			if (mkdir(mdir, PERMS) == -1) {
				tst_resm(TWARN, "mkdir(%s, %#o) failed",
					 mdir, PERMS);
				exit(1);
			}

			SAFE_TOUCH(cleanup, mname, 0700, NULL);

			/* rename "old" to "new" */
			TEST(rename(fname, mname));
			if (TEST_RETURN != -1) {
				tst_resm(TFAIL, "call succeeded unexpectedly");
				continue;
			}

			if (TEST_ERRNO != EACCES) {
				tst_resm(TFAIL, "Expected EACCES got %d",
					 TEST_ERRNO);
			} else {
				tst_resm(TPASS, "rename() returned EACCES");
			}

			/* set the process id back to root */
			if (seteuid(0) == -1) {
				tst_resm(TWARN, "seteuid(0) failed");
				exit(1);
			}

			/* clean up things in case we are looping */
			SAFE_UNLINK(cleanup, fname);
			SAFE_UNLINK(cleanup, mname);
			SAFE_RMDIR(cleanup, fdir);
			SAFE_RMDIR(cleanup, mdir);
		} else {
			/* parent - let the second child carry on */
			waitpid(pid1, &status, 0);
			if (!WIFEXITED(status) || (WEXITSTATUS(status) != 0)) {
				exit(WEXITSTATUS(status));
			} else {
				exit(0);
			}
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

	sprintf(fdir, "tdir_%d", getpid());
	sprintf(mdir, "rndir_%d", getpid());
	sprintf(fname, "%s/tfile_%d", fdir, getpid());
	sprintf(mname, "%s/rnfile_%d", mdir, getpid());
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
