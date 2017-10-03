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
 *	rename13
 *
 * DESCRIPTION
 *	Verify rename() return successfully and performs no other action
 *      when "old" file and "new" file link to the same file.
 *
 * ALGORITHM
 *	Setup:
 *		Setup signal handling.
 *		Create temporary directory.
 *		Pause for SIGUSR1 if option specified.
 *
 *	Test:
 *		Loop if the proper options are given.
 *                  create the "old" file
 *                  link the "new" file to the "old" file
 *                  rename the "old" to the "new" file
 *                  verify the "new" file points to the original file
 *                  verify the "old" file exists and points to
 *                         the original file
 *	Cleanup:
 *		Print errno log and/or timing stats if options given
 *		Delete the temporary directory created.
 *
 * USAGE
 *	rename13 [-c n] [-f] [-i n] [-I x] [-P x] [-t]
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
void cleanup();

char *TCID = "rename13";
int TST_TOTAL = 1;

int fd;
char fname[255], mname[255];
struct stat buf1, buf2;
dev_t olddev;
ino_t oldino;

int main(int ac, char **av)
{
	int lc;

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
		 * TEST rename()works when
		 * both old file and new file link to the same file
		 */

		/* Call rename(2) */
		TEST(rename(fname, mname));

		if (TEST_RETURN == -1) {
			tst_resm(TFAIL, "rename(%s, %s) failed", fname, mname);
			continue;
		}

		/* check the existence of "new", and get the status */
		SAFE_STAT(cleanup, mname, &buf2);

		/* check the existence of "old", and get the status */
		SAFE_STAT(cleanup, fname, &buf1);

		/* verify the new file is the same as the original */
		if (buf2.st_dev != olddev || buf2.st_ino != oldino) {
			tst_resm(TFAIL,
				 "rename() failed: new file does "
				 "not point to the same file as old "
				 "file");
			continue;
		}

		/* verify the old file is unchanged */
		if (buf1.st_dev != olddev || buf1.st_ino != oldino) {
			tst_resm(TFAIL,
				 "rename() failed: old file does "
				 "not point to the original file");
			continue;
		}

		tst_resm(TPASS, "functionality of rename() is correct");
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

	SAFE_TOUCH(cleanup, fname, 0700, NULL);

	SAFE_STAT(cleanup, fname, &buf1);

	/* save the dev and inode */
	olddev = buf1.st_dev;
	oldino = buf1.st_ino;

	/* link the "new" file to the "old" file */
	SAFE_LINK(cleanup, fname, mname);
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
