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
 *	rename12
 *
 * DESCRIPTION
 *      check rename() fails with EPERM or EACCES
 *
 * ALGORITHM
 *	Setup:
 *		Setup signal handling.
 *		Create temporary directory.
 *		Pause for SIGUSR1 if option specified.
 *
 *	Test:
 *		Loop if the proper options are given.
 *              create a directory fdir and set the sticky bit
 *              create file fname under fdir
 *              fork a child
 *                      set to nobody
 *                      try to rename fname to mname
 *                      check the return value, if succeeded (return=0)
 *			       Log the errno and Issue a FAIL message.
 *		        Otherwise,
 *			       Verify the errno
 *			       if equals to EPERMS or EACCES,
 *				       Issue Pass message.
 *			       Otherwise,
 *				       Issue Fail message.
 *	Cleanup:
 *		Print errno log and/or timing stats if options given
 *		Delete the temporary directory created.
 * USAGE
 *	rename12 [-c n] [-e] [-i n] [-I x] [-P x] [-t]
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
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <pwd.h>
#include <unistd.h>
#include "test.h"
#include "usctest.h"

void setup();
void cleanup();
extern void do_file_setup(char *);
extern struct passwd *my_getpwnam(char *);

#define PERMS		0777

char user1name[] = "nobody";
char user2name[] = "bin";

char *TCID = "rename12";	/* Test program identifier.    */
int TST_TOTAL = 1;		/* Total number of test cases. */

int fd;
char fdir[255];
char fname[255], mname[255];
struct passwd *nobody;
struct stat buf1;

int exp_enos[] = { EPERM, 0 };	/* List must end with 0 */

int main(int ac, char **av)
{
	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */
	pid_t pid;
	int status;

	/*
	 * parse standard options
	 */
	if ((msg = parse_opts(ac, av, NULL, NULL)) != NULL)
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);

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

		Tst_count = 0;

		/*
		 * rename a file whose parent directory has
		 * the sticky bit set without root permission
		 * or effective uid
		 */

		if ((pid = FORK_OR_VFORK()) == -1) {
			tst_brkm(TBROK, cleanup, "fork() failed");
		 }

		if (pid == 0) {	/* child */
			/* set to nobody */
			if (seteuid(nobody->pw_uid) == -1) {
				tst_resm(TWARN, "setreuid failed");
				perror("setreuid");
				exit(1);
			 }

			/* rename "old" to "new" */
			TEST(rename(fname, mname));

			if (TEST_RETURN != -1) {
				tst_resm(TFAIL, "call succeeded unexpectedly");
				exit(1);
			 }

			TEST_ERROR_LOG(TEST_ERRNO);

			if ((TEST_ERRNO != EPERM) && (TEST_ERRNO != EACCES)) {
				tst_resm(TFAIL,
					 "Expected EPERM or EACCES, got %d",
					 TEST_ERRNO);
				exit(1);
			 } else {
				tst_resm(TPASS,
					 "rename returned EPERM or EACCES");
			}

			/* set the id back to root */
			if (seteuid(0) == -1) {
				tst_resm(TWARN, "seteuid(0) failed");
			}
		} else {	/* parent */
			wait(&status);
			if (!WIFEXITED(status) || (WEXITSTATUS(status) != 0)) {
				exit(WEXITSTATUS(status));
			} else {
				exit(0);
			}

		}
	}

	cleanup();
	tst_exit();

}

/*
 * setup() - performs all ONE TIME setup for this test.
 */
void setup()
{
	/* must run as root */
	if (geteuid() != 0) {
		tst_brkm(TBROK, NULL, "Must run this as root");
	 }

	tst_sig(FORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;

	/* Create a temporary directory and make it current. */
	tst_tmpdir();

	umask(0);

	sprintf(fdir, "./tdir_%d", getpid());
	sprintf(fname, "%s/tfile_%d", fdir, getpid());
	sprintf(mname, "%s/rnfile_%d", fdir, getpid());

	/* create a directory */
	if (mkdir(fdir, PERMS) == -1) {
		tst_brkm(TBROK, cleanup, "Could not create directory %s", fdir);
	 }

	if (stat(fdir, &buf1) == -1) {
		tst_brkm(TBROK, cleanup, "failed to stat directory %s", fdir);

	}

	/* set the sticky bit */
	if (chmod(fdir, buf1.st_mode | S_ISVTX) != 0) {
		tst_brkm(TBROK, cleanup, "failed to set the S_ISVTX bit");

	}

	/* create a file under fdir */
	do_file_setup(fname);

	/* get nobody password file info */
	nobody = my_getpwnam(user1name);
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
}