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
 * DESCRIPTION
 *	Testcase to check open(2) sets errno to EACCES correctly.
 *
 * ALGORITHM
 *	Create a file owned by root with no read permission for other users.
 *	Attempt to open it as ltpuser(1). The attempt should fail with EACCES.
 * RESTRICTION
 *	Must run test as root.
 */
#include <errno.h>
#include <pwd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "test.h"
#include "safe_macros.h"

char *TCID = "open05";
int TST_TOTAL = 1;

static char fname[20];
static int fd;

static uid_t nobody_uid;

static void cleanup(void);
static void setup(void);

int main(int ac, char **av)
{
	int lc;
	int e_code, status, retval = 0;
	pid_t pid;

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		/* reset tst_count in case we are looping */
		tst_count = 0;

		pid = FORK_OR_VFORK();
		if (pid == -1)
			tst_brkm(TBROK, cleanup, "fork() failed");

		if (pid == 0) {
			if (seteuid(nobody_uid) == -1) {
				tst_resm(TWARN, "seteuid() failed, errno: %d",
					 errno);
			}

			TEST(open(fname, O_RDWR));

			if (TEST_RETURN != -1) {
				tst_resm(TFAIL, "open succeeded unexpectedly");
				continue;
			}

			if (TEST_ERRNO != EACCES) {
				retval = 1;
				tst_resm(TFAIL, "Expected EACCES got %d",
					 TEST_ERRNO);
			} else {
				tst_resm(TPASS, "open returned expected "
					 "EACCES error");
			}

			/* set the id back to root */
			if (seteuid(0) == -1)
				tst_resm(TWARN, "seteuid(0) failed");

			exit(retval);

		} else {
			/* wait for the child to finish */
			wait(&status);
			/* make sure the child returned a good exit status */
			e_code = status >> 8;
			if ((e_code != 0) || (retval != 0))
				tst_resm(TFAIL, "Failures reported above");

			close(fd);
			cleanup();

		}
	}

	tst_exit();
}

static void setup(void)
{
	struct passwd *pw;

	tst_require_root();

	pw = SAFE_GETPWNAM(NULL, "nobody");
	nobody_uid = pw->pw_uid;

	tst_sig(FORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;

	/* make a temporary directory and cd to it */
	tst_tmpdir();

	sprintf(fname, "file.%d", getpid());

	fd = SAFE_OPEN(cleanup, fname, O_RDWR | O_CREAT, 0700);
}

static void cleanup(void)
{
	unlink(fname);

	/* delete the test directory created in setup() */
	tst_rmdir();
}
