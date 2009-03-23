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
 *	rmdir01
 *
 * DESCRIPTION
 *	This test will verify that rmdir(2) syscall basic functionality.
 *	verify rmdir(2) returns a value of 0 and the directory being
 *	removed
 *
 * ALGORITHM
 *	Setup:
 *		Setup signal handling.
 *		Create temporary directory.
 *		Pause for SIGUSR1 if option specified.
 *
 *	Test:
 *		Loop if the proper options are given.
 *                 make a directory tstdir
 *                 call rmdir(tstdir), check the return value
 *                 verify the directory tstdir does not exists.
 *
 *	Cleanup:
 *		Print errno log and/or timing stats if options given
 *		Delete the temporary directory created.*
 * USAGE
 *	rmdir01 [-c n] [-f] [-i n] [-I x] [-P x] [-t]
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
#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include "test.h"
#include "usctest.h"

void setup();
void cleanup();

#define PERMS		0777

char *TCID = "rmdir01";		/* Test program identifier.    */
int TST_TOTAL = 1;		/* Total number of test cases. */
extern int Tst_count;		/* Test Case counter for tst_* routines */

char tstdir[100];

int main(int ac, char **av)
{
	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */
	struct stat buf;

	/*
	 * parse standard options
	 */
	if ((msg = parse_opts(ac, av, (option_t *) NULL, NULL)) != (char *)NULL) {
		tst_brkm(TBROK, tst_exit, "OPTION PARSING ERROR - %s", msg);
	}

	/*
	 * perform global setup for test
	 */
	setup();

	/*
	 * check looping state if -i option given
	 */
	for (lc = 0; TEST_LOOPING(lc); lc++) {

		/* reset Tst_count in case we are looping. */
		Tst_count = 0;

		/*
		 * TEST rmdir() base functionality
		 */

		/* Initialize the test directory name */

		/* create a directory */
		if (mkdir(tstdir, PERMS) == -1) {
			tst_brkm(TBROK, cleanup, "mkdir(%s, %#o) Failed",
				 tstdir, PERMS);
		 /*NOTREACHED*/}
		/* call rmdir using TEST macro */

		TEST(rmdir(tstdir));

		if (TEST_RETURN == -1) {
			tst_resm(TFAIL, "rmdir(%s) Failed", tstdir);
			continue;
		}

		if (STD_FUNCTIONAL_TEST) {
			/* check whether tstdir been removed */
			if (stat(tstdir, &buf) != -1) {
				tst_resm(TFAIL, "directory %s still exists",
					 tstdir);
				continue;
			} else {
				tst_resm(TPASS, "directory has been removed");
			}
		} else {
			tst_resm(TPASS, "call succeeded");
		}

	}			/* End for TEST_LOOPING */

	/*
	 * cleanup and exit
	 */
	cleanup();

	 /*NOTREACHED*/ return 0;

}

/*
 * setup() - performs all ONE TIME setup for this test.
 */
void setup()
{
	/* capture signals */
	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	/* Pause if that option was specified */
	TEST_PAUSE;

	/* Create a temporary directory and make it current. */
	tst_tmpdir();

	sprintf(tstdir, "./tstdir_%d", getpid());
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

	/*
	 * Exit with return code appropriate for results.
	 */
	tst_exit();
}
