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
#include "usctest.h"

void setup();
void cleanup();
extern void do_file_setup(char *);
extern struct passwd *my_getpwnam(char *);

#define PERMS		0700

char user1name[] = "nobody";
char user2name[] = "bin";

char *TCID = "rename09";	/* Test program identifier.    */
int TST_TOTAL = 1;		/* Total number of test cases. */
extern int Tst_count;		/* Test Case counter for tst_* routines */

char fdir[255], mdir[255];
char fname[255], mname[255];
struct passwd *nobody, *bin;

int exp_enos[] = { EACCES, 0 };	/* List must end with 0 */

int main(int ac, char **av)
{
	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */
	int rval;
	pid_t pid, pid1;
	int status;

	/*
	 * parse standard options
	 */
	if ((msg = parse_opts(ac, av, (option_t *) NULL, NULL)) != (char *)NULL) {
		tst_brkm(TBROK, tst_exit, "OPTION PARSING ERROR - %s", msg);
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

		if ((pid = FORK_OR_VFORK()) == -1) {
			tst_brkm(TBROK, cleanup, "fork() #1 failed");
		 /*NOTREACHED*/}

		if (pid == 0) {	/* first child */
			/* set to nobody */
			rval = setreuid(nobody->pw_uid, nobody->pw_uid);
			if (rval < 0) {
				tst_resm(TWARN, "setreuid failed to "
					 "to set the real uid to %d and "
					 "effective uid to %d",
					 nobody->pw_uid, nobody->pw_uid);
				perror("setreuid");
				exit(1);
			 /*NOTREACHED*/}

			/* create the a directory with 0700 permits */
			if (mkdir(fdir, PERMS) == -1) {
				tst_resm(TWARN, "mkdir(%s, %#o) Failed",
					 fdir, PERMS);
				exit(1);
			 /*NOTREACHED*/}

			/* create "old" file under it */
			do_file_setup(fname);

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
		 /*NOTREACHED*/}

		if (pid1 == 0) {	/* second child */
			/* set to bin */
			if ((rval = seteuid(bin->pw_uid)) == -1) {
				tst_resm(TWARN, "seteuid() failed");
				perror("setreuid");
				exit(1);
			 /*NOTREACHED*/}

			/* create "new" directory */
			if (mkdir(mdir, PERMS) == -1) {
				tst_resm(TWARN, "mkdir(%s, %#o) failed",
					 mdir, PERMS);
				exit(1);
			 /*NOTREACHED*/}

			/* create the new file */
			do_file_setup(mname);

			/* rename "old" to "new" */
			TEST(rename(fname, mname));
			if (TEST_RETURN != -1) {
				tst_resm(TFAIL, "call succeeded unexpectedly");
				continue;
			}

			TEST_ERROR_LOG(TEST_ERRNO);

			if (TEST_ERRNO != EACCES) {
				tst_resm(TFAIL, "Expected EACCES got %d",
					 TEST_ERRNO);
			 /*NOTREACHED*/} else {
				tst_resm(TPASS, "rename() returned EACCES");
			}

			/* set the process id back to root */
			if (seteuid(0) == -1) {
				tst_resm(TWARN, "seteuid(0) failed");
				exit(1);
			}

			/* clean up things in case we are looping */
			if (unlink(fname) == -1) {
				tst_brkm(TBROK, cleanup, "unlink() #1 failed");
			}
			if (unlink(mname) == -1) {
				tst_brkm(TBROK, cleanup, "unlink() #2 failed");
			}
			if (rmdir(fdir) == -1) {
				tst_brkm(TBROK, cleanup, "rmdir() #1 failed");
			}
			if (rmdir(mdir) == -1) {
				tst_brkm(TBROK, cleanup, "rmdir() #2 failed");
			}
		} else {
			/* parent - let the second child carry on */
			waitpid(pid1, &status, 0);
			if (!WIFEXITED(status) || (WEXITSTATUS(status) != 0)) {
				exit(WEXITSTATUS(status));
			} else {
				exit(0);
			}
		}
	}			/* End for TEST_LOOPING */

	/*
	 * cleanup and exit
	 */
	cleanup();

	 /*NOTREACHED*/ return 0;

}

/*
 * setup() - performs all ONE TIME setup for this test.
 */
void setup()
{
	/* must run as root */
	if (geteuid() != 0) {
		tst_brkm(TBROK, tst_exit, "Must run test as root");
	 /*NOTREACHED*/}

	/* capture signals */
	tst_sig(FORK, DEF_HANDLER, cleanup);

	/* Pause if that option was specified */
	TEST_PAUSE;

	/* Create a temporary directory and make it current. */
	tst_tmpdir();

	umask(0);

	sprintf(fdir, "tdir_%d", getpid());
	sprintf(mdir, "rndir_%d", getpid());
	sprintf(fname, "%s/tfile_%d", fdir, getpid());
	sprintf(mname, "%s/rnfile_%d", mdir, getpid());

	nobody = my_getpwnam(user1name);
	bin = my_getpwnam(user2name);
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
