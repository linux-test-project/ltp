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
 *    TEST IDENTIFIER   : flock02
 *
 *    EXECUTED BY       : anyone
 *
 *    TEST TITLE        : Error condition test for flock(2)
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
 * 	This test verifies that
 *	1)	flock(2) returns -1 and sets error number to EBADF
 *		if the file descriptor is invalid.
 *      2)	flock(2) returns -1 and sets error number to EINVAL
 *		if the argument operation does not include LOCK_SH,LOCK_EX,LOCK_UN.$
 *	3)	flock(2) returns -1 and sets error number to EINVAL
 *		if an invalid combination of locking modes is used i.e LOCK_SH with LOCK_EX
 *	
 *	Setup:
 *        Setup signal handling.
 *        Pause for SIGUSR1 if option specified.
 *        Create a temporary directory and chdir to it.
 * 	  Create a temporary file
 *
 *	Test:
 *	Loop if proper options are given.
 *		Execute system call
 *		Check return code,
 *			if system call failed (return == -1) and errno == expected_errno
 *				Issue system call fails with expected return value and error number
 *			else
 *				Issue system call failed to produce expected error.
 *	
 *      Cleanup:
 *        Print errno log and/or timing stats if options given
 *	  Deletes temporary directory.
 *
 * USAGE:  <for command-line>
 *      flock02 [-c n] [-e] [-i n] [-I x] [-P x] [-t] [-h] [-f] [-p]
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
int exp_enos[] = { EBADF, EINVAL, 0 };

char *TCID = "flock02";		/* Test program identifier */
int TST_TOTAL = 3;		/* Total number of test cases */
extern int Tst_count;
char filename[100];
int fd;

int main(int argc, char **argv)
{
	int lc;
	/* loop counter */
	char *msg;		/* message returned from parse_opts */

	/* parse standard options */
	if ((msg = parse_opts(argc, argv, (option_t *) NULL, NULL)) !=
	    (char *)NULL) {
		tst_brkm(TBROK, cleanup, "OPTION PARSING ERROR - %s", msg);
	 /*NOTREACHED*/}

	/* global setup */
	setup();

	/* The following loop checks looping state if -i option given */

	for (lc = 0; TEST_LOOPING(lc); lc++) {

		/* reset Tst_count in case we are looping */
		Tst_count = 0;

		/* Testing system call with negative file descriptor */
		TEST(flock(-1, LOCK_SH));

		if ((TEST_RETURN == -1) && (TEST_ERRNO == EBADF)) {
			tst_resm(TPASS,
				 "flock() shows expected failure,error number=%d : %s",
				 TEST_ERRNO, strerror(TEST_ERRNO));
		} else {
			tst_resm(TFAIL,
				 "flock() unexpectedly succeds, returned error number=%d",
				 TEST_ERRNO);
		}

		/* Test system call with invalid argument */
		TEST(flock(fd, LOCK_NB));

		if ((TEST_RETURN == -1) && (TEST_ERRNO == EINVAL)) {
			tst_resm(TPASS,
				 "flock() shows expected failure,error number=%d : %s",
				 TEST_ERRNO, strerror(TEST_ERRNO));

		} else {
			tst_resm(TFAIL,
				 "flock() unexpectedly succeds, returned error number=%d",
				 TEST_ERRNO);
		}

		/* Test system call with invalid combination of arguments */
		TEST(flock(fd, LOCK_SH | LOCK_EX));

		if ((TEST_RETURN == -1) && (TEST_ERRNO == EINVAL)) {
			tst_resm(TPASS,
				 "flock() shows expected failure with invalid combination of arguments, "
				 "error number=%d : %s", TEST_ERRNO,
				 strerror(TEST_ERRNO));
			continue;	/*next loop for MTKERNEL  */
		} else {
			tst_resm(TFAIL,
				 "flock() unexpectedly succeds, returned error number=%d",
				 TEST_ERRNO);
		}

	}			/* End of TEST_LOOPING */

	close(fd);

	cleanup();

	return 0;

}

/*
 * setup()
 *	performs all ONE TIME setup for this test
 */
void setup(void)
{
	/* capture signals */
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

	sprintf(filename, "flock02.%d", getpid());

	/* creating temporary file */
	fd = creat(filename, 0666);
	if (fd < 0) {
		tst_resm(TFAIL, "creating a new file failed");

		TEST_CLEANUP;

		/* Removing temp dir */
		tst_rmdir();

		/* exit with return code appropriate for result */
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

	/* exit with return code appropriate for results */
	tst_exit();
 /*NOTREACHED*/}
