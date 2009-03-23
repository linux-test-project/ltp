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
 *    TEST IDENTIFIER   : fdatasync01
 *
 *    EXECUTED BY       : Any user
 *
 *    TEST TITLE        : Basic test for fdatasync(2)
 *
 *    TEST CASE TOTAL   : 1
 *
 *    AUTHOR            : Madhu T L <madhu.tarikere@wipro.com>
 *
 *    SIGNALS
 *	Uses SIGUSR1 to pause before test if option set.
 *	(See the parse_opts(3) man page).
 *
 *    DESCRIPTION
 *	This is a Phase I test for the fdatasync(2) system call.
 *	It is intended to provide a limited exposure of the system call.
 *
 *	Setup:
 *	  Setup signal handling.
 *	  Pause for SIGUSR1 if option specified.
 *	  Create a temp directory and cd to it
 *	  Initialize filename and open it in write mode for each child process.
 *
 *	Test:
 *	 Loop if the proper options are given.
 *	  Execute system call
 *	  Check return code, if system call failed (return=-1)
 *		Issue FAIL message with errno.
 *	  Otherwise, Issue PASS message.
 *
 *	Cleanup:
 *	  Print errno log and/or timing stats if options given
 *	  Remove temporary directory and all files in it.
 *
 * USAGE:  <for command-line>
 *  fdatasync01 [-c n] [-e] [-f] [-h] [-i n] [-I x] [-p] [-P x] [-t]
 *		where,  -c n : Run n copies concurrently.
 *			-e   : Turn on errno logging.
 *			-f   : Turn off functional testing
 *			-h   : Show help screen
 *			-i n : Execute test n times.
 *			-I x : Execute test for x seconds.
 *			-p   : Pause for SIGUSR1 before starting
 *			-P x : Pause for x seconds between iterations.
 *			-t   : Turn on syscall timing.
 *
 ****************************************************************/
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include "test.h"
#include "usctest.h"

extern int Tst_count;		/* Test Case counter for tst_* routines */

static int fd;
static char filename[30];
static void setup(void);
static void cleanup(void);

char *TCID = "fdatasync01";	/* Test program identifier.    */
int TST_TOTAL = 1;		/* Total number of test cases. */

int main(int argc, char **argv)
{
	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */

	/* parse standard options */
	if ((msg = parse_opts(argc, argv, (option_t *) NULL, NULL)) !=
	    (char *)NULL) {
		tst_brkm(TBROK, tst_exit, "OPTION PARSING ERROR - %s", msg);
	}

	/* perform global setup for test */
	setup();

	/* check looping state if -i option is given */
	for (lc = 0; TEST_LOOPING(lc); lc++) {

		/* reset Tst_count in case we are looping */
		Tst_count = 0;

		/* Test the system call */
		TEST(fdatasync(fd));

		/* check return code */
		if (TEST_RETURN == -1) {
			tst_resm(TFAIL, "fdatasync() failed, errno=%d : %s",
				 TEST_ERRNO, strerror(TEST_ERRNO));
		} else {
			/* No Functional verification yet */
			tst_resm(TPASS, "fdatasync() successful");
		}
	}

	/* perform global cleanup and exit */
	cleanup();

	 /*NOTREACHED*/ return 0;

}				/* End main */

/* setup() - performs all ONE TIME setup for this test */
void setup(void)
{
	/* capture signals */
	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	/* Pause if that option was specified
	 * TEST_PAUSE contains the code to fork the test with the -c option.
	 */
	TEST_PAUSE;

	/* make a temp directory and cd to it */
	tst_tmpdir();

	/* Initialize unique filename for each child process */
	if (sprintf(filename, "fdatasync_%d", getpid()) <= 0) {
		tst_brkm(TBROK, cleanup, "Failed to initialize filename");
	 /*NOTREACHED*/}
	if ((fd = open(filename, O_CREAT | O_WRONLY, 0777)) == -1) {	//mode must be specified when O_CREATE is in the flag
		tst_brkm(TBROK, cleanup, "open() failed");
	 /*NOTREACHED*/}
	if ((write(fd, filename, strlen(filename) + 1)) == -1) {
		tst_brkm(TBROK, cleanup, "write() failed");
	 /*NOTREACHED*/}
}

/*
 * cleanup()
 *	performs all ONE TIME cleanup for this test at
 *	completion or premature exit
 */
void cleanup(void)
{
	/*
	 * print timing stats if that option was specified.
	 * print errno log if that option was specified.
	 */
	close(fd);

	TEST_CLEANUP;

	/* Remove tmp dir and all files in it */
	tst_rmdir();

	/* exit with return code appropriate for results */
	tst_exit();
 /*NOTREACHED*/}
