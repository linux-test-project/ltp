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
#include "test.h"
#include "usctest.h"
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

char *TCID = "chdir03";
int TST_TOTAL = 1;

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
	int lc;
	char *msg;

	pid_t pid, pid1;
	int status;

	if ((msg = parse_opts(ac, av, NULL, NULL)) != NULL)
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);

	setup();

	TEST_EXP_ENOS(exp_enos);

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		Tst_count = 0;

		if ((pid = FORK_OR_VFORK()) < 0) {
			tst_brkm(TBROK, cleanup, "first fork failed");
		}

		if (pid == 0) {
			if (setreuid(ltpuser1->pw_uid, ltpuser1->pw_uid) != 0) {
				perror("setreuid failed in child #1");
				exit(1);
			}
			if (mkdir(good_dir, 00700) != 0) {
				perror("mkdir failed in child #1");
				exit(1);
			}
			exit(0);
		}
		wait(&status);

		if ((pid1 = FORK_OR_VFORK()) < 0)
			tst_brkm(TBROK, cleanup, "second fork failed");

		if (pid1 == 0) {	/* second child */

			int rval;

			/*
			 * set the child's ID to ltpuser2 using seteuid()
			 * so that the ID can be changed back after the
			 * TEST call is made.
			 */
			if (seteuid(ltpuser2->pw_uid) != 0) {
				perror("setreuid failed in child #2");
				exit(1);
			}

			TEST(chdir(good_dir));

			if (TEST_RETURN != -1) {
				printf("call succeeded unexpectedly\n");
				rval = 1;
			} else if (TEST_ERRNO != EACCES) {
				printf("didn't get EACCES as expected; got ");
				rval = 1;
			} else {
				printf("got EACCES as expected\n");
				rval = 0;
			}
			/* Only really required with vfork. */
			if (seteuid(0) != 0) {
				perror("seteuid(0) failed");
				rval = 1;
			}

			exit(rval);

		} else {
			if (wait(&status) == -1)
				tst_brkm(TBROK|TERRNO, cleanup, "wait failed");
			if (!WIFEXITED(status) || WEXITSTATUS(status) != 0)
				tst_brkm(TBROK, cleanup,
				    "child exited abnormally");
		}
		if (rmdir(good_dir) == -1) {
			tst_brkm(TBROK|TERRNO, cleanup,
			    "rmdir(%s) failed", good_dir);
		}
	}

	cleanup();
	tst_exit();

}

void setup()
{
	char *cur_dir = NULL;

	tst_require_root(NULL);

	tst_sig(FORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;

	tst_tmpdir();

	if ((cur_dir = getcwd(cur_dir, 0)) == NULL)
		tst_brkm(TBROK|TERRNO, cleanup, "getcwd failed");

	sprintf(good_dir, "%s/%d", cur_dir, getpid());

	ltpuser1 = my_getpwnam(user1name);
	ltpuser2 = my_getpwnam(user2name);
}

void cleanup()
{
	TEST_CLEANUP;

	tst_rmdir();

}