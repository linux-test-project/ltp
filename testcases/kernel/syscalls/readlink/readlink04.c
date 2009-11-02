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
 * Test Name : readlink04
 *
 * Test Description :
 *  Verify that, readlink call will succeed to read the contents of the
 *  symbolic link if invoked by non-root user who is not the owner of the
 *  symbolic link.
 *
 * Expected Result:
 *  readlink() should return the contents of symbolic link path in the
 *  specified buffer on success.
 *
 * Algorithm:
 *  Setup:
 *   Setup signal handling.
 *   Create temporary directory.
 *   Pause for SIGUSR1 if option specified.
 *
 *  Test:
 *   Loop if the proper options are given.
 *   Execute system call
 *   Check return code, if system call failed (return=-1)
 *   	if errno set == expected errno
 *   		Issue sys call fails with expected return value and errno.
 *   	Otherwise,
 *		Issue sys call fails with unexpected errno.
 *   Otherwise,
 *	Issue sys call returns unexpected value.
 *
 *  Cleanup:
 *   Print errno log and/or timing stats if options given
 *   Delete the temporary directory(s)/file(s) created.
 *
 * Usage:  <for command-line>
 *  readlink04 [-c n] [-f] [-i n] [-I x] [-P x] [-t]
 *     where,  -c n : Run n copies concurrently.
 *             -f   : Turn off functionality Testing.
 *	       -i n : Execute test n times.
 *	       -I x : Execute test for x seconds.
 *	       -P x : Pause for x seconds between iterations.
 *	       -t   : Turn on syscall timing.
 *
 * HISTORY
 *	07/2001 Ported by Wayne Boyer
 *
 * RESTRICTIONS:
 *  This test should be executed by 'super-user' only.
 */
#include <stdlib.h>
#include <pwd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/fcntl.h>
#include <sys/wait.h>
#include <errno.h>
#include <string.h>
#include "test.h"
#include "usctest.h"

char TESTFILE[] = "./testfile\0";	/* name of file to create */
char SYMFILE[] = "slink_file\0";	/* name of symbolic link to create */
char creat_slink[] = "/creat_slink";	/* name of executable to execvp() */

char nobody[] = "nobody";
char bin[] = "bin";

#define MAX_SIZE	256

char *TCID = "readlink04";	/* Test program identifier.    */
int TST_TOTAL = 1;		/* Total number of test cases. */
extern int Tst_count;		/* Test Case counter for tst_* routines */

char *symfile_path;
int exp_val;			/* strlen of testfile */
char buffer[MAX_SIZE];		/* temporary buffer to hold symlink contents */

void setup();			/* Setup function for the test */
void cleanup();			/* Cleanup function for the test */

int main(int ac, char **av)
{
	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */

	/* Parse standard options given to run the test. */
	msg = parse_opts(ac, av, (option_t *) NULL, NULL);
	if (msg != (char *)NULL) {
		tst_brkm(TBROK, tst_exit, "OPTION PARSING ERROR - %s", msg);
	}

	/* Perform global setup for test */
	setup();

	/* Check looping state if -i option given */
	for (lc = 0; TEST_LOOPING(lc); lc++) {

		/* Reset Tst_count in case we are looping. */
		Tst_count = 0;

		/*
		 * Call readlink(2) to read the contents of
		 * symlink into a buffer.
		 */
		TEST(readlink(symfile_path, buffer, sizeof(buffer)));

		/* Check return code of readlink(2) */
		if (TEST_RETURN == -1) {
			tst_resm(TFAIL, "readlink() on %s failed, errno=%d : "
				 "%s", symfile_path, TEST_ERRNO,
				 strerror(TEST_ERRNO));
			continue;
		}

		/*
		 * Perform functional verification if test
		 * executed without (-f) option.
		 */
		if (STD_FUNCTIONAL_TEST) {
			/*
			 * Compare the return value of readlink()
			 * with the expected value which is the
			 * strlen() of testfile.
			 */
			if (TEST_RETURN == exp_val) {
				/* Check for the contents of buffer */
				if (memcmp(buffer, TESTFILE, exp_val) != 0) {
					tst_brkm(TFAIL, cleanup, "TESTFILE %s "
						 "and buffer contents %s "
						 "differ", TESTFILE, buffer);
				} else {
					tst_resm(TPASS, "readlink() "
						 "functionality on '%s' is "
						 "correct", SYMFILE);
				}
			} else {
				tst_resm(TFAIL, "readlink() return value %ld "
					 "doesn't match, Expected %d",
					 TEST_RETURN, exp_val);
			}
		} else {
			tst_resm(TPASS, "call succeeded");
		}
	}			/* End for TEST_LOOPING */
	/* Call cleanup() to undo setup done for the test. */
	cleanup();

	return 0;
}				/* End main */

/*
 * setup() - performs all ONE TIME setup for this test.
 *
 *  Create a temporary directory and change directory to it.
 *
 *  execvp() the creat_slink program as bin to creat a file and symlink.
 */
void setup()
{
	int pid;
	char *tmp_dir = NULL;
	char path_buffer[BUFSIZ];	/* Buffer to hold command string */
	char *cargv[4];
	char bin_dir[PATH_MAX];	/* variable to hold TESTHOME env */
	struct passwd *pwent;

	/* Check that the test process id is super/root  */
	if (geteuid() != 0) {
		tst_brkm(TBROK, tst_exit, "Must be root for this test!");
	}

	/* capture signals */
	tst_sig(FORK, DEF_HANDLER, cleanup);

	/* Pause if that option was specified */
	TEST_PAUSE;

	/* Get current bin directory */
	if (getcwd(bin_dir, sizeof(bin_dir)) == NULL) {
		tst_brkm(TBROK, tst_exit,
			 "getcwd(3) fails to get working directory of process");
	}

	/* make a temp directory and cd to it */
	tst_tmpdir();

	/* get the name of the temporary directory */
	if ((tmp_dir = getcwd(tmp_dir, 0)) == NULL) {
		tst_brkm(TBROK, tst_exit, "getcwd failed");
	}

	if ((pwent = getpwnam("bin")) == NULL) {
		tst_brkm(TBROK, cleanup, "getpwname() failed");
	}

	/* make the tmp directory belong to bin */
	if (chown(tmp_dir, pwent->pw_uid, pwent->pw_gid) == -1) {
		tst_brkm(TBROK, cleanup, "chown() failed");
	}

	if (chmod(tmp_dir, 0711) != 0) {
		tst_brkm(TBROK, cleanup, "chmod() failed");
	}

	/* create the full pathname of the executable to be execvp'ed */
	strcpy((char *)path_buffer, (char *)bin_dir);
	strcat((char *)path_buffer, (char *)creat_slink);

	symfile_path = "slink_file\0";

	/* set up the argument vector to pass into the execvp call */
	cargv[0] = tmp_dir;
	cargv[1] = TESTFILE;
	cargv[2] = symfile_path;
	cargv[3] = NULL;

	if ((pid = FORK_OR_VFORK()) == -1) {
		tst_brkm(TBROK, cleanup, "fork failed");
	}

	if (pid == 0) {		/* child */
		/*
		 * execvp the process/program that will create the test file
		 * and set up the symlink
		 */
		execvp(path_buffer, cargv);

		/* on success, execvp will not return */
		perror("execvp");
		tst_brkm(TBROK, NULL, "execvp() failed");

		/*
		 * In reality, the contents/functionality of the creat_slink
		 * program could be included right here.  This would simplify
		 * the test a bit.  For now, however, we'll leave it as is.
		 */
	}

	/* parent */

	/* wait to let the execvp'ed process do its work */
	waitpid(pid, NULL, 0);

	/* set up the expected return value from the readlink() call */
	exp_val = strlen(TESTFILE);

	/* fill the buffer with a known value */
	(void)memset(buffer, 0, MAX_SIZE);

	/* finally, change the id of the parent process to "nobody" */
	if ((pwent = getpwnam("nobody")) == NULL) {
		tst_brkm(TBROK, cleanup, "getpwname() failed for nobody");
	}

	if (seteuid(pwent->pw_uid) == -1) {
		tst_brkm(TBROK, cleanup, "seteuid() failed for nobody");
	}
}

/*
 * cleanup() - performs all ONE TIME cleanup for this test at
 *             completion or premature exit.
 *
 *  Remove the test directory and testfile created in the setup.
 */
void cleanup()
{
	/*
	 * print timing stats if that option was specified.
	 * print errno log if that option was specified.
	 */
	TEST_CLEANUP;

	/* set the process id back to root in order to remove the tmp dir */
	if (seteuid(0) == -1) {
		tst_brkm(TBROK, NULL, "failed to set process id to root");
	}

	/* Remove tmp dir and all files in it */
	tst_rmdir();

	/* exit with return code appropriate for results */
	tst_exit();
}
