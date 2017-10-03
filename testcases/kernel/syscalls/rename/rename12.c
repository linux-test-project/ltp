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
#include "safe_macros.h"

void setup();
void cleanup();

#define PERMS		0777

char *TCID = "rename12";
int TST_TOTAL = 1;

int fd;
char fdir[255];
char fname[255], mname[255];
uid_t nobody_uid;
struct stat buf1;

int main(int ac, char **av)
{
	int lc;
	pid_t pid;
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
			if (seteuid(nobody_uid) == -1) {
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
void setup(void)
{
	struct passwd *pw;

	tst_require_root();

	tst_sig(FORK, DEF_HANDLER, cleanup);

	pw = SAFE_GETPWNAM(NULL, "nobody");
	nobody_uid = pw->pw_uid;

	TEST_PAUSE;

	/* Create a temporary directory and make it current. */
	tst_tmpdir();

	umask(0);

	sprintf(fdir, "./tdir_%d", getpid());
	sprintf(fname, "%s/tfile_%d", fdir, getpid());
	sprintf(mname, "%s/rnfile_%d", fdir, getpid());

	/* create a directory */
	SAFE_MKDIR(cleanup, fdir, PERMS);

	SAFE_STAT(cleanup, fdir, &buf1);

	/* set the sticky bit */
	if (chmod(fdir, buf1.st_mode | S_ISVTX) != 0) {
		tst_brkm(TBROK, cleanup, "failed to set the S_ISVTX bit");

	}

	/* create a file under fdir */
	SAFE_TOUCH(cleanup, fname, 0700, NULL);
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
}
