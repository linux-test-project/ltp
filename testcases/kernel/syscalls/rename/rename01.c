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
 *	rename01
 *
 * DESCRIPTION
 *	This test will verify the rename(2) syscall basic functionality.
 *	Verify rename() works when the "new" file or directory does not exist.
 *
 * ALGORITHM
 *	Setup:
 *		Setup signal handling.
 *		Create temporary directory.
 *		Pause for SIGUSR1 if option specified.
 *
 *	Test:
 *		Loop if the proper options are given.
 *              1.  "old" is plain file, new does not exists
 *                  create the "old" file, make sure the "new" file
 *                  dose not exist
 *                  rename the "old" to the "new" file
 *                  verify the "new" file points to the "old" file
 *                  verify the "old" file does not exist
 *
 *              2.  "old" is a directory,"new" does not exists
 *                  create the "old" directory, make sure "new"
 *                  dose not exist
 *                  rename the "old" to the "new"
 *                  verify the "new" points to the "old"
 *                  verify the "old" does not exist
 *	Cleanup:
 *		Print errno log and/or timing stats if options given
 *		Delete the temporary directory created.
 *
 * USAGE
 *	rename01 [-c n] [-f] [-i n] [-I x] [-P x] [-t]
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

char *TCID = "rename01";	/* Test program identifier.    */
int TST_TOTAL = 2;		/* Total number of test cases. */

char fname[255], mname[255];
char fdir[255], mdir[255];
struct stat buf1;
dev_t f_olddev, d_olddev;
ino_t f_oldino, d_oldino;

struct test_case_t {
	char *name1;
	char *name2;
	char *desc;
	dev_t *olddev;
	ino_t *oldino;
} TC[] = {
	/* comment goes here */
	{
	fname, mname, "file", &f_olddev, &f_oldino},
	    /* comment goes here */
	{
	fdir, mdir, "directory", &d_olddev, &d_oldino}
};

int main(int ac, char **av)
{
	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */
	int i;

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

		/* loop through the test cases */
		for (i = 0; i < TST_TOTAL; i++) {

			TEST(rename(TC[i].name1, TC[i].name2));

			if (TEST_RETURN == -1) {
				tst_resm(TFAIL, "call failed unexpectedly");
				continue;
			}

			if (STD_FUNCTIONAL_TEST) {
				if (stat(TC[i].name2, &buf1) == -1) {
					tst_brkm(TBROK, cleanup, "stat of %s "
						 "failed", TC[i].desc);

				}

				/*
				 * verify the new file or directory is the
				 * same as the old one
				 */
				if (buf1.st_dev != *TC[i].olddev ||
				    buf1.st_ino != *TC[i].oldino) {
					tst_resm(TFAIL, "rename() failed: the "
						 "new %s points to a different "
						 "inode/location", TC[i].desc);
					continue;
				}
				/*
				 * verify that the old file or directory
				 * does not exist
				 */
				if (stat(fname, &buf1) != -1) {
					tst_resm(TFAIL, "the old %s still "
						 "exists", TC[i].desc);
					continue;
				}

				tst_resm(TPASS, "functionality is correct "
					 "for renaming a %s", TC[i].desc);
			} else {
				tst_resm(TPASS, "call succeeded on %s rename",
					 TC[i].desc);
			}
		}
		/* reset things in case we are looping */
		if (rename(mname, fname) == -1) {
			tst_brkm(TBROK, cleanup, "file rename failed");
		}

		if (rename(mdir, fdir) == -1) {
			tst_brkm(TBROK, cleanup, "directory rename failed");
		}
	}

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
	sprintf(fdir, "./tdir_%d", getpid());
	sprintf(mdir, "./rndir_%d", getpid());

	/* create the "old" file */
	do_file_setup(fname);

	if (stat(fname, &buf1) == -1) {
		tst_brkm(TBROK, cleanup, "failed to stat file %s"
			 "in setup()", fname);

	}

	f_olddev = buf1.st_dev;
	f_oldino = buf1.st_ino;

	/* create "old" directory */
	if (mkdir(fdir, 00770) == -1) {
		tst_brkm(TBROK, cleanup, "Could not create directory %s", fdir);
	 }

	if (stat(fdir, &buf1) == -1) {
		tst_brkm(TBROK, cleanup, "failed to stat directory %s"
			 "in setup()", fname);

	}

	d_olddev = buf1.st_dev;
	d_oldino = buf1.st_ino;
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