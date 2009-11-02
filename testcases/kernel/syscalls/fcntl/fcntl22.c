/*
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
 * Test Name: fcntl22
 *
 * Test Description:
 *  Verify that, fcntl() fails with -1 and sets errno to EAGAIN when
 *				  Operation  is  prohibited  by locks held by other processes.
 *
 * Expected Result:
 *  fcntl() should fail with return value -1 and sets expected errno.
 *
 * Algorithm:
 *  Setup:
 *   Setup signal handling.
 *   Pause for SIGUSR1 if option specified.
 *   Create temporary directory.
 *   Create and open temporary file.
 *   Lock temporary file.
 *
 *  Test:
 *   Loop if the proper options are given.
 *   Duplicate process
 *   Parent waits for child termination
 *   Child execute system call
 *   Check return code, if system call failed (return=-1)
 *		 if errno set == expected errno
 *				 Issue sys call fails with expected return value and errno.
 *		 Otherwise,
 *				 Issue sys call fails with unexpected errno.
 *   Otherwise,
 *		 Issue sys call returns unexpected value.
 *
 *  Cleanup:
 *   Print errno log and/or timing stats if options given
 *
 * Usage:  <for command-line>
 *  fcntl22 [-c n] [-e] [-f] [-i n] [-I x] [-P x] [-t]
 *     where,  -c n : Run n copies concurrently.
 *             -e   : Turn on errno logging.
 *             -f   : Turn off functionality Testing.
 *		        -i n : Execute test n times.
 *		        -I x : Execute test for x seconds.
 *		        -P x : Pause for x seconds between iterations.
 *		        -t   : Turn on syscall timing.
 *
 * HISTORY
 *		 06/2002 Ported by Jacky Malcles
 *
 * RESTRICTIONS:
 *  none.
 *
 */

#include <fcntl.h>
#include <errno.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "test.h"
#include "usctest.h"

int child_pid;
int file;
struct flock fl;		/* struct flock for fcntl */

#ifdef __hpux
/* oddball HP-UX declares the error case to be EACCES in the man page */
int exp_enos[] = { EACCES, 0 };
#else
int exp_enos[] = { EAGAIN, 0 };
#endif

char *TCID = "fcntl22";
int TST_TOTAL = 1;
extern int Tst_count;

void setup(void);
void cleanup(void);

int main(int ac, char **av)
{
	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */
	char *test_desc;	/* test specific error message */

	/* parse standard options */
	if ((msg = parse_opts(ac, av, (option_t *) NULL, NULL)) != (char *)NULL) {
		tst_brkm(TBROK, cleanup, "OPTION PARSING ERROR - %s", msg);
	}

	/* setup */
	setup();

	/* set the expected errnos... */
	TEST_EXP_ENOS(exp_enos);

	/* Check for looping state if -i option is given */
	for (lc = 0; TEST_LOOPING(lc); lc++) {
#ifdef __hpux
		test_desc = "EACCES";
#else
		test_desc = "EAGAIN";
#endif

		/* reset Tst_count in case we are looping */
		Tst_count = 0;

		/* duplicate process */
		if ((child_pid = FORK_OR_VFORK()) == 0) {
			/* child */
			/*
			 * Call fcntl(2) with F_SETLK   argument on file
			 */
			TEST(fcntl(file, F_SETLK, &fl));

			/* Check return code from fcntl(2) */
			if (TEST_RETURN != -1) {
				tst_resm(TFAIL, "fcntl() returned %ld,"
					 "expected -1, errno=%d", TEST_RETURN,
					 exp_enos[0]);
			} else {
				TEST_ERROR_LOG(TEST_ERRNO);

				if (TEST_ERRNO == exp_enos[0]) {
					tst_resm(TPASS,
						 "fcntl() fails with expected "
						 "error %s errno:%d", test_desc,
						 TEST_ERRNO);
				} else {
					tst_resm(TFAIL, "fcntl() fails, %s, "
						 "errno=%d, expected errno=%d",
						 test_desc, TEST_ERRNO,
						 exp_enos[0]);
				}
			}
			/* end child */
		} else {
			if (child_pid < 0) {
				/* quit if fork fail */
				tst_resm(TFAIL, "Fork failed");
				cleanup();
			} else {
				/* Parent waits for child termination */
				wait(0);
				cleanup();
			}
		}

	}			/* end for */
	return 0;
}

/*
 * setup
 *		 performs all ONE TIME setup for this test
 */
void setup()
{

	/* capture signals */
	tst_sig(FORK, DEF_HANDLER, cleanup);

	/* Pause if that option was specified */
	TEST_PAUSE;

	/* Make a temp dir and cd to it */
	tst_tmpdir();

	/* create regular file */
	if ((file = creat("regfile", 0777)) == -1) {
		tst_brkm(TBROK, cleanup,
			 "creat(regfile, 0777) failed, errno:%d %s", errno,
			 strerror(errno));
	}
	/* struct lock */
	fl.l_type = F_WRLCK;
	fl.l_whence = 0;
	fl.l_start = 0;
	fl.l_len = 0;

	/* lock file */
	if (fcntl(file, F_SETLK, &fl) < 0) {
		tst_resm(TFAIL|TERRNO, "fcntl on file %d failed",
			 file);
	}

}

/*
 * cleanup()
 *		 performs all ONE TIME cleanup for this test at completion or
 *		 premature exit
 */
void cleanup()
{
	/*
	 * print timing stats if that option was specified
	 * print errno log if that option was specified
	 */
	close(file);

	TEST_CLEANUP;
	/* remove temporary directory */
	tst_rmdir();

	/* exit with return code appropriate for results */
	tst_exit();
}
