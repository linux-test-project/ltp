/*
 * Copyright (c) International Business Machines  Corp., 2001
 *  07/2001 Ported by Wayne Boyer
 *
 * This program is free software;  you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY;  without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
 * the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program;  if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

/*
 * DESCRIPTION
 *	This test will verify the mkdir(2) syscall basic functionality
 */

#include <errno.h>
#include <string.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <pwd.h>
#include "test.h"

void setup();
void cleanup();

#define PERMS		0777

char *TCID = "mkdir05";
int TST_TOTAL = 1;

char nobody_uid[] = "nobody";
struct passwd *ltpuser;

char tstdir1[100];

int main(int ac, char **av)
{
	int lc;
	struct stat buf;

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
		 * TEST mkdir() base functionality
		 */

		/* Initialize the test directory name */
		sprintf(tstdir1, "tstdir1.%d", getpid());

		/* Call mkdir(2) using the TEST macro */
		TEST(mkdir(tstdir1, PERMS));

		if (TEST_RETURN == -1) {
			tst_resm(TFAIL, "mkdir(%s, %#o) Failed",
				 tstdir1, PERMS);
			continue;
		}

		if (stat(tstdir1, &buf) == -1) {
			tst_brkm(TBROK, cleanup, "failed to stat the "
				 "new directory");
		}
		/* check the owner */
		if (buf.st_uid != geteuid()) {
			tst_resm(TFAIL, "mkdir() FAILED to set owner ID"
				 " as process's effective ID");
			continue;
		}
		/* check the group ID */
		if (buf.st_gid != getegid()) {
			tst_resm(TFAIL, "mkdir() failed to set group ID"
				 " as the process's group ID");
			continue;
		}
		tst_resm(TPASS, "mkdir() functionality is correct");

		/* clean up things in case we are looping */
		if (rmdir(tstdir1) == -1) {
			tst_brkm(TBROK, cleanup, "could not remove directory");
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
	tst_require_root();

	ltpuser = getpwnam(nobody_uid);
	if (setuid(ltpuser->pw_uid) == -1) {
		tst_resm(TINFO, "setuid failed to "
			 "to set the effective uid to %d", ltpuser->pw_uid);
		perror("setuid");
	}

	tst_sig(FORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;

	/* Create a temporary directory and make it current. */
	tst_tmpdir();
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
