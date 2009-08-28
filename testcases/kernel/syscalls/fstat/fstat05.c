/*
 *
 *   Copyright (C) Bull S.A. 2001
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
 * Test Name: fstat05
 *
 * Test Description:
 *   Verify that,
 *   if buffer points outside user's accessible address space fstat(2)
 *	either returns -1 and sets errno to EFAULT
 *	or SIGSEGV is returned instead of EFAULT
 *
 * Expected Result:
 *   fstat() should fail with return value -1 and set expected errno.
 *    or
 *   fstat() should fail with SIGSEGV returned.
 *   Both results are considered as acceptable.
 *
 * Algorithm:
 *  Setup:
 *   Setup signal handling SIGSEGV included.
 *   Switch to nobody user.
 *   Pause for SIGUSR1 if option specified.
 *   Create temporary directory.
 *   Create a testfile under temporary directory.
 *
 *  Test:
 *   Buffer points outside user's accessible address space.
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
 *  Sighandler:
 *	if signal == SIGSEGV
 *		Issue sys call fails with expected signal
 *      Otherwise,
 *              Issue sys call fails with unexpected signal.
 *
 *  Cleanup:
 *   Print errno log and/or timing stats if options given
 *   Close the test file
 *   Delete the temporary directory(s)/file(s) created.
 *
 * Usage:  <for command-line>
 *  fstat05 [-c n] [-e] [-i n] [-I x] [-p x] [-t]
 *	where,  -c n : Run n copies concurrently.
 *		-e   : Turn on errno logging.
 *		-i n : Execute test n times.
 *		-I x : Execute test for x seconds.
 *		-P x : Pause for x seconds between iterations.
 *		-t   : Turn on syscall timing.
 *
 * History
 *	05/2002 Jacky Malcles
 *		-Ported
 *
 * Restrictions:
 *      This test must be run as root.
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
#include <pwd.h>

#include "test.h"
#include "usctest.h"

#define TEST_FILE       "testfile"

char nobody_uid[] = "nobody";
struct passwd *ltpuser;
extern struct passwd *my_getpwnam(char *);

char *TCID = "fstat05";		/* Test program identifier.    */
int TST_TOTAL = 1;		/* Total number of test cases. */
extern int Tst_count;		/* Test Case counter for tst_* routines */
int exp_enos[] = { EFAULT, 0 };
int fildes;			/* testfile descriptor */

void setup();			/* Main setup function for the tests */
void cleanup();			/* cleanup function for the test */
void sighandler(int sig);	/* signals handler function for the test */

int siglist[] = { SIGHUP, SIGINT, SIGQUIT, SIGILL, SIGTRAP, SIGABRT, SIGIOT,
	SIGBUS, SIGFPE, SIGUSR1, SIGSEGV, SIGUSR2, SIGPIPE, SIGALRM,
	SIGTERM,
#ifdef SIGSTKFLT
	SIGSTKFLT,
#endif
	SIGCHLD, SIGCONT, SIGTSTP, SIGTTIN,
	SIGTTOU, SIGURG, SIGXCPU, SIGXFSZ, SIGVTALRM, SIGPROF,
	SIGWINCH, SIGIO, SIGPWR, SIGSYS,
#ifdef SIGUNUSED
	SIGUNUSED
#endif
};

int SIG_SEEN = sizeof(siglist) / sizeof(int);

#if !defined(UCLINUX)

int main(int ac, char **av)
{
	struct stat stat_buf;	/* stat structure buffer */
	struct stat *ptr_str;
	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */

	/* Parse standard options given to run the test. */
	msg = parse_opts(ac, av, (option_t *) NULL, NULL);
	if (msg != (char *)NULL) {
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);
		tst_exit();
	 /*NOTREACHED*/}

	/* Buffer points outside user's accessible address space. */
	ptr_str = &stat_buf;	/* if it was for conformance testing */
	ptr_str = (void *)sbrk(0) + (4 * getpagesize());

	/*
	 * Invoke setup function
	 */
	setup();

	/* set the expected errnos... */
	TEST_EXP_ENOS(exp_enos);

	/* Check looping state if -i option given */
	for (lc = 0; TEST_LOOPING(lc); lc++) {
		/* Reset Tst_count in case we are looping. */
		Tst_count = 0;

		/*
		 * Call fstat(2).
		 * verify that it fails with -1 return value and
		 * sets appropriate errno.
		 */
		TEST(fstat(fildes, ptr_str));

		/* Check return code from fstat(2) */
		if (TEST_RETURN == -1) {
			TEST_ERROR_LOG(TEST_ERRNO);
			if (TEST_ERRNO == EFAULT) {
				tst_resm(TPASS, "fstat() fails with "
					 "expected error EFAULT");
			} else {
				tst_resm(TFAIL, "fstat() fails with "
					 "wrong errno:%d", TEST_ERRNO);
			}
		} else {
			tst_resm(TFAIL, "fstat() returned %ld but we wanted -1",
				 TEST_RETURN);
		}

	}			/* End of TEST CASE LOOPING. */

	/*
	 * Invoke cleanup() to delete the test directory/file(s) created
	 * in the setup().
	 */
	cleanup();
	 /*NOTREACHED*/ return 0;
}				/* End main */

#else

int main()
{
	tst_resm(TINFO, "test is not available on uClinux");
	return 0;
}

#endif /* if !defined(UCLINUX) */

/*
 * void
 * setup(void) - performs all ONE TIME setup for this test.
 *	Exit the test program on receipt of unexpected signals.
 *	Create a temporary directory and change directory to it.
 */
void setup()
{
	int i;

	/*
	 * Capture unexpected signals SIGSEGV included
	 * SIGSEGV being considered as acceptable as returned value
	 */
	for (i = 0; i < SIG_SEEN; i++) {

		signal(siglist[i], &sighandler);
	}

	/* Switch to nobody user for correct error code collection */
	if (geteuid() != 0) {
		tst_brkm(TBROK, tst_exit, "Test must be run as root");
	}

	ltpuser = getpwnam(nobody_uid);
	if (setuid(ltpuser->pw_uid) == -1)
		tst_resm(TINFO, "setuid(%d) failed", ltpuser->pw_uid);

	/* Pause if that option was specified
	 * TEST_PAUSE contains the code to fork the test with the -i option.
	 * You want to make sure you do this before you create your temporary
	 * directory.
	 */
	TEST_PAUSE;

	/* Make a temp dir and cd to it */
	tst_tmpdir();

	/* Create a testfile under temporary directory */
	fildes = open(TEST_FILE, O_RDWR | O_CREAT, 0666);
	if (fildes == -1)
		tst_brkm(TBROK|TERRNO, cleanup,
			 "open(%s, O_RDWR|O_CREAT, 0666) failed", TEST_FILE);

}				/* End setup() */

/*
 * void
 * cleanup() - Performs all ONE TIME cleanup for this test at
 *             completion or premature exit.
 *	Print test timing stats and errno log if test executed with options.
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

	if (close(fildes) == -1)
		tst_brkm(TBROK|TERRNO, cleanup, "close(%s) failed", TEST_FILE);

	/* Remove files and temporary directory created */
	tst_rmdir();

	/* exit with return code appropriate for results */
	tst_exit();
}				/* End cleanup() */

/*
 * sighandler() - handle the signals
 */

void sighandler(int sig)
{
	if (sig == SIGSEGV) {
		tst_resm(TPASS, "fstat() fails with "
			 "expected signal SIGSEGV");
	} else {
		tst_brkm(TBROK, 0, "Unexpected signal %d received.", sig);
	}
	cleanup();
	/*NOT REACHED */

}
