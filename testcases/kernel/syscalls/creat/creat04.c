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
 *	creat04.c
 *
 * DESCRIPTION
 *	testcase to check creat(2) fails with EACCES
 *
 * ALGORITHM
 *	1.	A parent spawns a child, which creates a test-directory, and
 *		makes it 700. Then the parent spawns another child, does a
 *		setreuid to ltpuser1, and attempts to creat() a file (which
 *		does not exist) in the directory created by the first child.
 *	2.	Repeat 1, for a file (to be created by ltpuser1) which exists.
 *		That is the first child should creat() a file before exiting.
 *
 * USAGE:  <for command-line>
 *  creat04 [-c n] [-e] [-i n] [-I x] [-P x] [-t]
 *     where,  -c n : Run n copies concurrently.
 *             -e   : Turn on errno logging.
 *             -i n : Execute test n times.
 *             -I x : Execute test for x seconds.
 *             -P x : Pause for x seconds between iterations.
 *             -t   : Turn on syscall timing.
 *
 * HISTORY
 *	07/2001 Ported by Wayne Boyer
 *
 * RESTRICTIONS
 *	Test must be run as root.
 */

#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <pwd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>

#include "test.h"
#include "safe_macros.h"

char *TCID = "creat04";
int TST_TOTAL = 2;

void setup(void);
void cleanup(void);

#define FMODE	0444
#define DMODE	00700

static char fname_dir[] = "testdir";
static char fname[] = "testdir/file";
static char fname1[] = "testdir/file1";

static uid_t nobody_uid;

struct test_case_t {
	char *fname;
} TC[] = {
	{
	fname}, {
	fname1}
};

int main(int ac, char **av)
{
	int lc;
	int retval = 0;

	pid_t pid, pid1;
	int i, status, fd;

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {

		/* reset tst_count in case we are looping */
		tst_count = 0;

		if ((pid = FORK_OR_VFORK()) == -1) {
			tst_brkm(TBROK | TERRNO, cleanup, "fork() #1 failed");
		}

		if (pid == 0) {	/* first child */
			if (mkdir(fname_dir, DMODE) != 0) {
				perror("mkdir() failed");
				exit(1);
			}
			if ((fd = open(fname1, O_RDWR | O_CREAT, 0444)) == -1) {
				perror("open failed");
				exit(1);
			}
			exit(0);
		}
		wait(&status);

		/* make sure the child returned a good exit status */
		if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
			tst_brkm(TBROK, cleanup, "child #1 failed");
		}

		if ((pid1 = FORK_OR_VFORK()) == -1) {
			tst_brkm(TBROK | TERRNO, cleanup, "fork() #2 failed");
		}

		if (pid1 == 0) {	/* second child */
			if (seteuid(nobody_uid) == -1) {
				perror("seteuid");
				exit(1);
			}

			/* loop through the test cases */
			for (i = 0; i < TST_TOTAL; i++) {

				TEST(creat(TC[i].fname, FMODE));

				if (TEST_RETURN != -1) {
					retval = 1;
					tst_resm(TFAIL, "call succeeded "
						 "unexpectedly");
					continue;
				}

				if (TEST_ERRNO != EACCES) {
					retval = 1;
					tst_resm(TFAIL | TTERRNO,
						 "Expected EACCES");
				} else {
					tst_resm(TPASS, "call failed with "
						 "EACCES as expected");
				}
			}

			seteuid(0);

			/* clean up things in case we are looping */
			unlink(fname);
			unlink(fname1);
			rmdir(fname_dir);
			exit(retval);

		} else {	/* parent */
			/* wait for the child to finish */
			wait(&status);
			/* make sure the child returned a good exit status */
			if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
				tst_resm(TFAIL, "see failures reported above");
			}
		}
	}
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

	tst_sig(FORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;

	/* make a temporary directory and cd to it */
	tst_tmpdir();
}

/*
 * cleanup() - performs all ONE TIME cleanup for this test at
 *	       completion or premature exit.
 */
void cleanup(void)
{

	/* delete the test directory created in setup() */
	tst_rmdir();

}
