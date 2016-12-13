/*
 * Copyright (c) International Business Machines  Corp., 2001
 * Copyright (c) 2013 Cyril Hrubis <chrubis@suse.cz>
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
 * along with this program;  if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
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
 */

#include <stdlib.h>
#include <pwd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <errno.h>
#include <string.h>
#include "test.h"

char *TCID = "readlink04";
int TST_TOTAL = 1;

static char *TESTFILE = "./testfile";
static char *SYMFILE = "slink_file";

#define MAX_SIZE	256

static int exp_val;
static char buffer[MAX_SIZE];

static void setup(void);
static void cleanup(void);

int main(int ac, char **av)
{
	int lc;

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {

		tst_count = 0;

		/*
		 * Call readlink(2) to read the contents of
		 * symlink into a buffer.
		 */
		TEST(readlink(SYMFILE, buffer, sizeof(buffer)));

		if (TEST_RETURN == -1) {
			tst_resm(TFAIL | TTERRNO, "readlink() on %s failed",
			         SYMFILE);
			continue;
		}

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
	}

	cleanup();
	tst_exit();
}

#define FILE_MODE  S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH

static void setup(void)
{
	int fd;
	char *tmp_dir = NULL;
	struct passwd *pwent;

	tst_require_root();

	TEST_PAUSE;

	tst_tmpdir();

	/* get the name of the temporary directory */
	if ((tmp_dir = getcwd(tmp_dir, 0)) == NULL)
		tst_brkm(TBROK, NULL, "getcwd failed");

	if ((pwent = getpwnam("bin")) == NULL)
		tst_brkm(TBROK, cleanup, "getpwname() failed");

	/* make the tmp directory belong to bin */
	if (chown(tmp_dir, pwent->pw_uid, pwent->pw_gid) == -1)
		tst_brkm(TBROK, cleanup, "chown() failed");

	if (chmod(tmp_dir, 0711) != 0)
		tst_brkm(TBROK|TERRNO, cleanup, "chmod(%s) failed", tmp_dir);

	/* create test file and symlink */
	if ((fd = open(TESTFILE, O_RDWR | O_CREAT, FILE_MODE)) == -1)
		tst_brkm(TBROK|TERRNO, cleanup, "open(%s) failed", TESTFILE);

	if (close(fd))
		tst_brkm(TBROK|TERRNO, cleanup, "close(%s) failed", TESTFILE);

	if (symlink(TESTFILE, SYMFILE) < 0) {
		tst_brkm(TBROK|TERRNO, cleanup, "symlink(%s, %s) failed",
		         TESTFILE, SYMFILE);
	}

	/* set up the expected return value from the readlink() call */
	exp_val = strlen(TESTFILE);

	/* fill the buffer with a known value */
	memset(buffer, 0, MAX_SIZE);

	/* finally, change the id of the parent process to "nobody" */
	if ((pwent = getpwnam("nobody")) == NULL)
		tst_brkm(TBROK, cleanup, "getpwname() failed for nobody");

	if (seteuid(pwent->pw_uid) == -1)
		tst_brkm(TBROK, cleanup, "seteuid() failed for nobody");
}

static void cleanup(void)
{
	if (seteuid(0) == -1)
		tst_brkm(TBROK, NULL, "failed to set process id to root");

	tst_rmdir();
}
