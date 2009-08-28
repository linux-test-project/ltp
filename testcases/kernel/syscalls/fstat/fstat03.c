/*
 * Test Name: fstat03
 *
 * Test Description:
 *   Verify that, fstat(2) returns -1 and sets errno to EBADF if the file
 *   pointed to by file descriptor is not valid.
 *
 * Expected Result:
 *  fstat() should fail with return value -1 and set expected errno.
 *
 * Algorithm:
 *  Setup:
 *   Setup signal handling.
 *   Create temporary directory.
 *   Pause for SIGUSR1 if option specified.
 *
 *  Test:
 *   Loop if the proper options are given.
 *   Execute system call
 *   Check return code, if system call failed (return=-1)
 *	if errno set == expected errno
 *		Issue sys call fails with expected return value and errno.
 *	Otherwise,
 *		Issue sys call fails with unexpected errno.
 *   Otherwise,
 *	Issue sys call returns unexpected value.
 *
 *  Cleanup:
 *   Print errno log and/or timing stats if options given
 *   Delete the temporary directory(s)/file(s) created.
 *
 * Usage:  <for command-line>
 *  fstat03 [-c n] [-e] [-i n] [-I x] [-P x] [-t]
 *     where,  -c n : Run n copies concurrently.
 *             -e   : Turn on errno logging.
 *	       -i n : Execute test n times.
 *	       -I x : Execute test for x seconds.
 *	       -P x : Pause for x seconds between iterations.
 *	       -t   : Turn on syscall timing.
 *
 * HISTORY
 *	07/2001 Ported by Wayne Boyer
 *
 * RESTRICTIONS:
 *  This test should be executed by 'non-super-user' only.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "test.h"
#include "usctest.h"

#define FILE_MODE	S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH
#define TEST_FILE	"testfile"

char *TCID = "fstat03";		/* Test program identifier.    */
int TST_TOTAL = 1;		/* Total number of test cases. */
extern int Tst_count;		/* Test Case counter for tst_* routines */
int exp_enos[] = { EBADF, 0 };
int fildes;			/* testfile descriptor */

void setup();			/* Main setup function for the tests */
void cleanup();			/* cleanup function for the test */

int main(int ac, char **av)
{
	struct stat stat_buf;	/* stat structure buffer */
	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */

	/* Parse standard options given to run the test. */
	msg = parse_opts(ac, av, (option_t *) NULL, NULL);
	if (msg != (char *)NULL) {
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);
		tst_exit();
	}

	/*
	 * Invoke setup function to create a testfile under temporary
	 * directory.
	 */
	setup();

	/* set the expected errnos... */
	TEST_EXP_ENOS(exp_enos);

	/* Check looping state if -i option given */
	for (lc = 0; TEST_LOOPING(lc); lc++) {
		/* Reset Tst_count in case we are looping. */
		Tst_count = 0;
		/*
		 * Call fstat(2) to get the status information
		 * of a closed testfile pointed to by 'fd'.
		 * verify that fstat fails with -1 return value and
		 * sets appropriate errno.
		 */
		TEST(fstat(fildes, &stat_buf));

		/* Check return code from fstat(2) */
		if (TEST_RETURN == -1) {
			TEST_ERROR_LOG(TEST_ERRNO);
			if (TEST_ERRNO == EBADF) {
				tst_resm(TPASS, "fstat() fails with expected error EBADF");
			} else {
				tst_resm(TFAIL|TTERRNO, "fstat() did not fail with EBADF");
			}
		} else {
			tst_resm(TFAIL, "fstat() returned %ld, expected -1", TEST_RETURN);
		}
	}			/* End for TEST_LOOPING */

	/*
	 * Invoke cleanup() to delete the test directory/file(s) created
	 * in the setup().
	 */
	cleanup();

	 /*NOTREACHED*/ return 0;
}				/* End main */

/*
 * void
 * setup(void) - performs all ONE TIME setup for this test.
 *	Exit the test program on receipt of unexpected signals.
 *	Create a temporary directory and change directory to it.
 *      Create a testfile under temporary directory.
 *      Close the testfile.
 */
void setup()
{
	/* Capture unexpected signals */
	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	/* Pause if that option was specified */
	TEST_PAUSE;

	/* Make a temp dir and cd to it */
	tst_tmpdir();

	/* Create a testfile under temporary directory */
	fildes = open(TEST_FILE, O_RDWR | O_CREAT, 0666);
	if (fildes == -1)
		tst_brkm(TBROK|TERRNO, cleanup,
			 "open(%s, O_RDWR|O_CREAT, 0666) failed", TEST_FILE);

	if (close(fildes) == -1)
		tst_brkm(TBROK|TERRNO, cleanup, "close(%s) failed", TEST_FILE);
}				/* End of setup */

/*
 * void
 * cleanup() - Performs all ONE TIME cleanup for this test at
 *             completion or premature exit.
 *	Print test timing stats and errno log if test executed with options.
 *	Close the testfile if still opened.
 *	Remove temporary directory and sub-directories/files under it
 *	created during setup().
 *	Exit the test program with normal exit code.
 */
void cleanup()
{
	/*
	 * print timing stats if that option was specified.
	 * print errno log if that option was specified.
	 */
	TEST_CLEANUP;

	/* Remove files and temporary directory created */
	tst_rmdir();

	/* exit with return code appropriate for results */
	tst_exit();
}				/* End cleanup() */
