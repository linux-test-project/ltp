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
 * with this program; if not, write the Free Software Foundation, Inc., 59
 * Temple Place - Suite 330, Boston MA 02111-1307, USA.
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
 * 	Test to verify flock(2) succeds with all kind of locks.
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
#include "test.h"
#include "usctest.h"

void setup(void);
void cleanup(void);

/* 0 terminated list of expected errnos */
int exp_enos[] = { EWOULDBLOCK, EAGAIN, EINVAL, 0 };

char *TCID = "flock01";		/* Test program identifier */
int TST_TOTAL = 3;		/* Total number of test cases */
char filename[100];
int fd;

struct test_case_t {
	int operation;
	char *opt;
} test_cases[] = {
	{
	LOCK_SH, "Shared Lock"}, {
	LOCK_UN, "Unlock"}, {
LOCK_EX, "Exclusive Lock"},};

int main(int argc, char **argv)
{
	int lc, i;
	/* loop counter */
	char *msg;		/* message returned from parse_opts */

	/* parse standard options */
	if ((msg = parse_opts(argc, argv, NULL, NULL)) !=
	    NULL) {
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);
	 }

	/* global setup */
	setup();

	/* The following loop checks looping state if -i option given */

	for (lc = 0; TEST_LOOPING(lc); lc++) {

		/* reset Tst_count in case we are looping */
		Tst_count = 0;

		for (i = 0; i < TST_TOTAL; ++i) {

			/* Testing system call */
			TEST(flock(fd, test_cases[i].operation));

			if (TEST_RETURN == -1) {
				TEST_ERROR_LOG(TEST_ERRNO);
				tst_resm(TFAIL,
					 "flock() failed to get %s, error number=%d : %s",
					 test_cases[i].opt, TEST_ERRNO,
					 strerror(TEST_ERRNO));
				continue;	/*next loop for MTKERNEL  */
			} else {
				tst_resm(TPASS,
					 "flock() succeded with %s, returned error number=%d",
					 test_cases[i].opt, TEST_ERRNO);
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

	/* Set up the expected error numbers for -e option */
	TEST_EXP_ENOS(exp_enos);

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
	fd = creat(filename, 0644);
	if (fd < 0) {
		tst_resm(TFAIL, "creating a new file failed");

		TEST_CLEANUP;

		/* Removing temp directory */
		tst_rmdir();

		/* exit with resturn code appropriate for result */
		tst_exit();

	}
}

/*
 * cleanup()
 *	performs all ONE TIME cleanup for this test at
 * 	completion or premature exit
 */
void cleanup(void)
{
	/*
	 * print timing stats if that option was specified.
	 * print errno log if that option was specified.
	 */
	TEST_CLEANUP;

	unlink(filename);
	tst_rmdir();

 }