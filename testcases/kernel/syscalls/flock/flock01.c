/*
 * Copyright (c) Wipro Technologies Ltd, 2002.  All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it would be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 */
/**********************************************************
 *
 *    TEST IDENTIFIER   : flock01
 *
 *    EXECUTED BY       : anyone
 *
 *    TEST TITLE        : Basic test for flock(2)
 *
 *    TEST CASE TOTAL   : 3
 *
 *    AUTHOR            : Vatsal Avasthi <vatsal.avasthi@wipro.com>
 *
 *    SIGNALS
 *      Uses SIGUSR1 to pause before test if option set.
 *      (See the parse_opts(3) man page).
 *
 *    DESCRIPTION
 * 	Test to verify flock(2) succeeds with all kind of locks.
 *	Intends to provide a limited exposure of system call.
 *    $
 *	Setup:
 *        Setup signal handling.
 *        Pause for SIGUSR1 if option specified.
 *        Create a temporary directory and chdir to it.
 * 	  Create a temporary file
 *
 *	Test:
 *	Loop if proper options are given.
 *		Execute system call
 *		Check return code, if system call failed (return == -1)
 *				Log the error number and issue a FAIL message
 *		otherwise issue a PASS message
 *
 *      Cleanup:
 *        Print errno log and/or timing stats if options given
 *	  Deletes temporary directory.
 *
 * USAGE:  <for command-line>
 *      flock01 [-c n] [-e] [-i n] [-I x] [-P x] [-t] [-h] [-f] [-p]
 *                      where,  -c n : Run n copies concurrently.
 *                              -f   : Turn off functional testing
 *    				-e   : Turn on errno logging.
 *                              -h   : Show help screen                        $
 *				-i n : Execute test n times.
 *                              -I x : Execute test for x seconds.
 *                              -p   : Pause for SIGUSR1 before starting
 *                              -P x : Pause for x seconds between iterations.
 *                              -t   : Turn on syscall timing.
 *
 ****************************************************************/

#include <errno.h>
#include <stdio.h>
#include <sys/wait.h>
#include <sys/file.h>
#include <fcntl.h>
#include "test.h"

void setup(void);
void cleanup(void);

char *TCID = "flock01";
int TST_TOTAL = 3;
char filename[100];
int fd;

struct test_case_t {
	int operation;
	char *opt;
} test_cases[] = {
	{ LOCK_SH, "Shared Lock" },
	{ LOCK_UN, "Unlock"},
	{ LOCK_EX, "Exclusive Lock"}
};

int main(int argc, char **argv)
{
	int lc, i;

	tst_parse_opts(argc, argv, NULL, NULL);

	/* global setup */
	setup();

	/* The following loop checks looping state if -i option given */

	for (lc = 0; TEST_LOOPING(lc); lc++) {

		/* reset tst_count in case we are looping */
		tst_count = 0;

		for (i = 0; i < TST_TOTAL; ++i) {

			/* Testing system call */
			TEST(flock(fd, test_cases[i].operation));
			if (TEST_RETURN == -1) {
				tst_resm(TFAIL | TTERRNO,
					 "flock() failed to get %s",
					 test_cases[i].opt);
				continue;	/*next loop for MTKERNEL  */
			} else {
				tst_resm(TPASS,
					 "flock() succeeded with %s",
					 test_cases[i].opt);
			}

		}

	}

	close(fd);

	cleanup();

	tst_exit();

}

/*
 * setup()
 *	performs all ONE TIME setup for this test
 */
void setup(void)
{

	tst_sig(FORK, DEF_HANDLER, cleanup);

	/* Pause if that option was specified
	 * TEST_PAUSE contains the code to fork the test with the -i option.
	 * You want to make sure you do this before you create your temporary
	 * directory.
	 */
	TEST_PAUSE;

	/* Create a unique temporary directory and chdir() to it. */
	tst_tmpdir();

	sprintf(filename, "flock01.%d", getpid());

	/* creating temporary file */
	fd = open(filename, O_CREAT | O_TRUNC | O_RDWR, 0644);
	if (fd < 0)
		tst_brkm(TBROK, tst_rmdir, "creating a new file failed");
}

/*
 * cleanup()
 *	performs all ONE TIME cleanup for this test at
 * 	completion or premature exit
 */
void cleanup(void)
{

	unlink(filename);
	tst_rmdir();

}
