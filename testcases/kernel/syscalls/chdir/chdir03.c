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
 *	chdir03
 *
 * DESCRIPTION
 *	Testcase for testing that chdir(2) sets EACCES errno
 *
 * ALGORITHM
 *	1.	create a child process, sets its uid to ltpuser1
 *	2.	this child creates a directory with perm 700, and exits
 *	3.	create another child process, sets its uid to ltpuser2
 *	4.	this child attempts to chdir(2) to the directory created in 2.
 *		and expects to get an EACCES.
 *
 * USAGE:  <for command-line>
 *  chdir03 [-c n] [-e] [-i n] [-I x] [-P x] [-t]
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
 *	This test must be run as root.
 */

#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <pwd.h>
#include <errno.h>
#include <sys/wait.h>
#include <test.h>
#include <usctest.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

char *TCID = "chdir03";
int TST_TOTAL = 1;
extern int Tst_count;

void setup(void);
void cleanup(void);

char user1name[] = "nobody";
char user2name[] = "bin";

int exp_enos[] = { EACCES, 0 };

char good_dir[100];

struct passwd *ltpuser1, *ltpuser2;

extern struct passwd *my_getpwnam(char *);

int main(int ac, char **av)
{
	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */

	pid_t pid, pid1;
	int status;

	/* parse standard options */
	if ((msg = parse_opts(ac, av, (option_t *) NULL, NULL)) != (char *)NULL) {
		tst_brkm(TBROK, cleanup, "OPTION PARSING ERROR - %s", msg);
	}

	setup();

	/* set up the expected errnos */
	TEST_EXP_ENOS(exp_enos);

	/* check for looping state if -i option is given */
	for (lc = 0; TEST_LOOPING(lc); lc++) {
		/* reset Tst_count in case we are looping */
		Tst_count = 0;

		if ((pid = FORK_OR_VFORK()) < 0) {
			tst_brkm(TBROK, cleanup, "first fork failed");
		}

		if (pid == 0) {	/* first child */
			/* set the child's ID to ltpuser1 */
			if (setreuid(ltpuser1->pw_uid, ltpuser1->pw_uid) != 0) {
				tst_resm(TINFO, "setreuid failed in child #1");
				exit(1);
			}
			if (mkdir(good_dir, 00700) != 0) {
				tst_resm(TINFO, "mkdir failed in child #1");
				exit(1);
			}
			exit(0);
		}
		wait(&status);

		if ((pid1 = FORK_OR_VFORK()) < 0) {
			tst_brkm(TBROK, cleanup, "second fork failed");
		}

		if (pid1 == 0) {	/* second child */
			/*
			 * set the child's ID to ltpuser2 using seteuid()
			 * so that the ID can be changed back after the
			 * TEST call is made.
			 */
			if (seteuid(ltpuser2->pw_uid) != 0) {
				tst_resm(TINFO, "setreuid failed in child #2");
				exit(1);
			}

			TEST(chdir(good_dir));

			if (TEST_RETURN != -1) {
				tst_resm(TFAIL, "call succeeded unexpectedly");
			} else if (TEST_ERRNO != EACCES) {
				tst_resm(TFAIL|TTERRNO, "expected EACCES");
			} else {
				TEST_ERROR_LOG(TEST_ERRNO);
				tst_resm(TPASS|TTERRNO, "expected failure");
			}

			/* reset the process ID to the saved ID (root) */
			if (setuid(0) == -1) {
				tst_resm(TINFO|TERRNO, "setuid(0) failed");
			}

		} else {	/* parent */
			wait(&status);

			/* let the child carry on */
			exit(0);
		}

		/* clean up things in case we are looping */
		if (rmdir(good_dir) == -1) {
			tst_brkm(TBROK|TERRNO, cleanup, "rmdir(%s) failed", good_dir);
		}
	}
	cleanup();

	return 0;
 /*NOTREACHED*/}

/*
 * setup() - performs all ONE TIME setup for this test.
 */
void setup()
{
	char *cur_dir = NULL;

	/* make sure the process ID is root */
	if (geteuid() != 0) {
		tst_brkm(TBROK, tst_exit, "Test must be run as root.");
	}

	/* capture signals */
	tst_sig(FORK, DEF_HANDLER, cleanup);

	/* Pause if that option was specified */
	TEST_PAUSE;

	/* make a temporary directory and cd to it */
	tst_tmpdir();

	/* get the currect directory name */
	if ((cur_dir = getcwd(cur_dir, 0)) == NULL) {
		tst_brkm(TBROK|TERRNO, cleanup, "Couldn't get current directory name");
	}

	sprintf(good_dir, "%s/%d", cur_dir, getpid());

	ltpuser1 = my_getpwnam(user1name);
	ltpuser2 = my_getpwnam(user2name);
}

/*
 * cleanup() - performs all ONE TIME cleanup for this test at
 *	       completion or premature exit.
 */
void cleanup()
{
	/*
	 * print timing stats if that option was specified.
	 * print errno log if that option was specified.
	 */
	TEST_CLEANUP;

	/*
	 * Delete the test directory created in setup().
	 */
	tst_rmdir();

	/* exit with return code appropriate for results */
	tst_exit();
}
