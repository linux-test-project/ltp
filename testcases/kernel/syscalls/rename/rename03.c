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
 *	rename03
 *
 * DESCRIPTION
 *	This test will verify that rename(2) functions correctly
 *	when the "new" file or directory exists
 *
 * ALGORITHM
 *	Setup:
 *		Setup signal handling.
 *		Create temporary directory.
 *		Pause for SIGUSR1 if option specified.
 *
 *	Test:
 *		Loop if the proper options are given.
 *              1.  both old and new file exist
 *                  create the "old" file and the "new" file
 *                  rename the "old" to the "new" file
 *                  verify the "new" file points to the "old" file
 *                  verify the "old" file does not exists
 *              2.  both old file and new directory exist
 *                  create the "old" and the "new" directory
 *                  rename the "old" to the "new" directory
 *                  verify the "new" points to the "old" directory
 *                  verify the "old" does not exists
 *	Cleanup:
 *		Print errno log and/or timing stats if options given
 *		Delete the temporary directory created.
 *
 * USAGE
 *	rename03 [-c n] [-f] [-i n] [-I x] [-p x] [-t]
 *	where,  -c n : Run n copies concurrently.
 *		-f   : Turn off functionality Testing.
 *		-i n : Execute test n times.
 *		-I x : Execute test for x seconds.
 *		-P x : Pause for x seconds between iterations.
 *		-t   : Turn on syscall timing.
 *
 * HISTORY
 *	07/2001 Ported by Wayne Boyer
 *
 * RESTRICTIONS
 *	None.
 */
#include <sys/types.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>

#include "test.h"
#include "safe_macros.h"

void setup();
void setup2();
void cleanup();

char *TCID = "rename03";
int TST_TOTAL = 2;

char fname[255], mname[255];
char fdir[255], mdir[255];
struct stat buf1, buf2;
dev_t f_olddev, d_olddev;
ino_t f_oldino, d_oldino;

struct test_case_t {
	char *name1;
	char *name2;
	char *desc;
	dev_t *olddev;
	ino_t *oldino;
} TC[] = {
	{
	fname, mname, "file", &f_olddev, &f_oldino}, {
	fdir, mdir, "directory", &d_olddev, &d_oldino}
};

int main(int ac, char **av)
{
	int lc;
	int i;

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

		/* set up the files and directories for the tests */
		setup2();

		/* loop through the test cases */
		for (i = 0; i < TST_TOTAL; i++) {

			TEST(rename(TC[i].name1, TC[i].name2));

			if (TEST_RETURN == -1) {
				tst_resm(TFAIL, "call failed unexpectedly");
				continue;
			}

			SAFE_STAT(cleanup, TC[i].name2, &buf2);

			/*
			 * verify the new file or directory is the
			 * same as the old one
			 */
			if (buf2.st_dev != *TC[i].olddev ||
			    buf2.st_ino != *TC[i].oldino) {
				tst_resm(TFAIL, "rename() failed: the "
					 "new %s points to a different "
					 "inode/location", TC[i].desc);
				continue;
			}
			/*
			 * verify that the old file or directory
			 * does not exist
			 */
			if (stat(fname, &buf2) != -1) {
				tst_resm(TFAIL, "the old %s still "
					 "exists", TC[i].desc);
				continue;
			}

			tst_resm(TPASS, "functionality is correct "
				 "for renaming a %s", TC[i].desc);
		}

		/* reset things in case we are looping */

		/* unlink the new file */
		SAFE_UNLINK(cleanup, mname);

		/* remove the new directory */
		SAFE_RMDIR(cleanup, mdir);
	}

	cleanup();
	tst_exit();

}

/*
 * setup() - performs all ONE TIME setup for this test.
 */
void setup(void)
{

	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;

	/* Create a temporary directory and make it current. */
	tst_tmpdir();

	sprintf(fname, "./tfile_%d", getpid());
	sprintf(mname, "./rnfile_%d", getpid());
	sprintf(fdir, "./tdir_%d", getpid());
	sprintf(mdir, "./rndir_%d", getpid());
}

/*
 * setup2() - set up the files and directories for the tests
 */
void setup2(void)
{
	SAFE_TOUCH(cleanup, fname, 0700, NULL);

	SAFE_STAT(cleanup, fname, &buf1);

	/* save original file's dev and ino */
	f_olddev = buf1.st_dev;
	f_oldino = buf1.st_ino;

	SAFE_TOUCH(cleanup, mname, 0700, NULL);

	/* create "old" directory */
	SAFE_MKDIR(cleanup, fdir, 00770);
	SAFE_STAT(cleanup, fdir, &buf1);

	d_olddev = buf1.st_dev;
	d_oldino = buf1.st_ino;

	/* create another directory */
	SAFE_MKDIR(cleanup, mdir, 00770);
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
