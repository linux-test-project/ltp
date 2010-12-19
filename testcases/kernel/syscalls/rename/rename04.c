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
 *	rename04
 *
 * DESCRIPTION
 *	This test will verify that rename(2) failed when newpath is
 *      a non-empty directory and return EEXIST or ENOTEMPTY
 *
 * ALGORITHM
 *	Setup:
 *		Setup signal handling.
 *		Create temporary directory.
 *		Pause for SIGUSR1 if option specified.
 *              create the "old" directory and the "new" directory
 *              create a file uner the "new" directory
 *
 *	Test:
 *		Loop if the proper options are given.
 *                  rename the "old" to the "new" directory
 *                  verify rename() failed and returned ENOTEMPTY
 *
 *	Cleanup:
 *		Print errno log and/or timing stats if options given
 *		Delete the temporary directory created.
 *
 * USAGE
 *	rename04 [-c n] [-e] [-i n] [-I x] [-P x] [-t]
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

char *TCID = "rename04";	/* Test program identifier.    */
int TST_TOTAL = 1;		/* Total number of test cases. */

int exp_enos[] = { ENOTEMPTY, EEXIST, 0 };	/* List must end with 0 */

int fd;
char tstfile[40];
char fdir[255], mdir[255];
struct stat buf1, buf2;
dev_t olddev, olddev1;
ino_t oldino, oldino1;

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

	/* set the expected errnos... */
	TEST_EXP_ENOS(exp_enos);

	/*
	 * check looping state if -i option given
	 */
	for (lc = 0; TEST_LOOPING(lc); lc++) {

		Tst_count = 0;

		/* rename a directory to a non-empty directory */

		/* Call rename(2) */
		TEST(rename(fdir, mdir));

		if (TEST_RETURN != -1) {
			tst_resm(TFAIL, "rename(%s, %s) succeeded unexpectedly",
				 fdir, mdir);
			continue;
		}

		TEST_ERROR_LOG(TEST_ERRNO);

		if (TEST_ERRNO == ENOTEMPTY) {
			tst_resm(TPASS, "rename() returned ENOTEMPTY");
		} else if (TEST_ERRNO == EEXIST) {
			tst_resm(TPASS, "rename() returned EEXIST");
		} else {
			tst_resm(TFAIL, "Expected ENOTEMPTY or EEXIST got %d",
				 TEST_ERRNO);
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

	sprintf(fdir, "./tdir_%d", getpid());
	sprintf(mdir, "./rndir_%d", getpid());
	sprintf(tstfile, "%s/tstfile_%d", mdir, getpid());

	/* create "old" directory */
	if (mkdir(fdir, 00770) == -1) {
		tst_brkm(TBROK, cleanup, "Could not create directory %s", fdir);
	 }

	if (stat(fdir, &buf1) == -1) {
		tst_brkm(TBROK, cleanup, "failed to stat directory %s"
			 "in rename()", fdir);

	}

	/* save "old"'s dev and ino */
	olddev = buf1.st_dev;
	oldino = buf1.st_ino;

	/* create another directory */
	if (mkdir(mdir, 00770) == -1) {
		tst_brkm(TBROK, cleanup, "Could not create directory %s", mdir);
	 }

	/* create a file under "new" directory */
	do_file_setup(tstfile);

	if (stat(mdir, &buf2) == -1) {
		tst_brkm(TBROK, cleanup, "failed to stat directory %s "
			 "in rename()", mdir);

	}

	/* save "new"'s dev and ino */
	olddev1 = buf2.st_dev;
	oldino1 = buf2.st_ino;
}

/*
 * cleanup() - performs all ONE TIME cleanup for this test at
 *              completion or premature exit.
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