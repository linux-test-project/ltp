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
#include <sys/fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>

#include "test.h"
#include "usctest.h"

void setup();
void cleanup();
extern void do_file_setup(char *);

char *TCID = "rename13";	/* Test program identifier.    */
int TST_TOTAL = 1;		/* Total number of test cases. */

int fd;
char fname[255], mname[255];
struct stat buf1, buf2;
dev_t olddev;
ino_t oldino;

int main(int ac, char **av)
{
	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */

	/*
	 * parse standard options
	 */
	if ((msg = parse_opts(ac, av, NULL, NULL)) != NULL)
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);

	/*
	 * perform global setup for test
	 */
	setup();

	/*
	 * check looping state if -i option given
	 */
	for (lc = 0; TEST_LOOPING(lc); lc++) {

		Tst_count = 0;

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

		if (STD_FUNCTIONAL_TEST) {
			/* check the existence of "new", and get the status */
			if (stat(mname, &buf2) == -1) {
				tst_brkm(TBROK, cleanup, "failed to stat file "
					 "%s in rename()", mname);

			}

			/* check the existence of "old", and get the status */
			if (stat(fname, &buf1) == -1) {
				tst_brkm(TBROK, cleanup, "failed to stat file "
					 "%s in rename()", fname);

			}

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
		} else {
			tst_resm(TPASS, "call succeeded");
		}
	}

	/*
	 * cleanup and exit
	 */
	cleanup();
	tst_exit();

}

/*
 * setup() - performs all ONE TIME setup for this test.
 */
void setup()
{

	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;

	/* Create a temporary directory and make it current. */
	tst_tmpdir();

	sprintf(fname, "./tfile_%d", getpid());
	sprintf(mname, "./rnfile_%d", getpid());

	/* create the "old" file */
	do_file_setup(fname);

	if (stat(fname, &buf1) == -1) {
		tst_brkm(TBROK, cleanup, "failed to stat file %s"
			 "in rename()", fname);

	}

	/* save the dev and inode */
	olddev = buf1.st_dev;
	oldino = buf1.st_ino;

	/* link the "new" file to the "old" file */
	if (link(fname, mname) == -1) {
		tst_brkm(TBROK, cleanup,
			 "link from %s to %s failed!", fname, mname);
	 }
}

/*
 * cleanup() - performs all ONE TIME cleanup for this test at
 *             completion or premature exit.
 */
void cleanup()
{
	/*
	 * print timing stats if that option was specified.
	 * print errno log if that option was specified.
	 */
	TEST_CLEANUP;

	/*
	 * Remove the temporary directory.
	 */
	tst_rmdir();

}
