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
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "test.h"
#include "safe_macros.h"

char *TCID = "chdir03";
int TST_TOTAL = 1;

void setup(void);
void cleanup(void);

char good_dir[100];

static uid_t nobody_uid, bin_uid;

int main(int ac, char **av)
{
	int lc;

	pid_t pid, pid1;
	int status;

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		tst_count = 0;

		if ((pid = FORK_OR_VFORK()) < 0) {
			tst_brkm(TBROK, cleanup, "first fork failed");
		}

		if (pid == 0) {
			if (setreuid(nobody_uid, nobody_uid) != 0) {
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
			if (seteuid(bin_uid) != 0) {
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
				tst_brkm(TBROK | TERRNO, cleanup,
					 "wait failed");
			if (!WIFEXITED(status) || WEXITSTATUS(status) != 0)
				tst_brkm(TBROK, cleanup,
					 "child exited abnormally");
			tst_resm(TPASS, "child reported success");
		}
		if (rmdir(good_dir) == -1) {
			tst_brkm(TBROK | TERRNO, cleanup,
				 "rmdir(%s) failed", good_dir);
		}
	}

	cleanup();
	tst_exit();
}

void setup(void)
{
	struct passwd *pw;
	char *cur_dir = NULL;

	tst_require_root();

	pw = SAFE_GETPWNAM(NULL, "nobody");
	nobody_uid = pw->pw_uid;
	pw = SAFE_GETPWNAM(NULL, "bin");
	bin_uid = pw->pw_uid;

	tst_sig(FORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;

	tst_tmpdir();

	if ((cur_dir = getcwd(cur_dir, 0)) == NULL)
		tst_brkm(TBROK | TERRNO, cleanup, "getcwd failed");

	sprintf(good_dir, "%s/%d", cur_dir, getpid());
}

void cleanup(void)
{
	tst_rmdir();

}
