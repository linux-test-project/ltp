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
 * NAME
 *      readdir02.c
 *
 * DESCRIPTION
 *      readdir02: try to readdir with Invalid directory stream descriptor dir.
 *
 * CALLS
 *      readdir(3)
 *
 * ALGORITHM
 *      loop if that option was specified
 *      call readdir() with an invalid file descriptor
 *      check the errno value
 *        issue a PASS message if we get EBADF - errno 9
 *      otherwise, the tests fails
 *        issue a FAIL message
 *        call cleanup
 *
 * USAGE:  <for command-line>
 *  readdir02 [-c n] [-e] [-i n] [-I x] [-P x] [-t]
 *     where,  -c n : Run n copies concurrently.
 *             -e   : Turn on errno logging.
 *             -i n : Execute test n times.
 *             -I x : Execute test for x seconds.
 *             -P x : Pause for x seconds between iterations.
 *             -t   : Turn on syscall timing.
 *
 * NOTE
 *	The POSIX standard says:
 *	  The readdir() function may fail if:
 *	  [EBADF] The dirp argument does not refer to an open directory stream.
 *	  (Note that readdir() is not _required_ to fail in this case.)
 *
 * HISTORY
 *      04/2002 - Written by Jacky Malcles
 *
 *      06/2003 - Added code to catch SIGSEGV and return TCONF.
 *		Robbie Williamson<robbiew@us.ibm.com>
 *
 * RESTRICTIONS
 *      none
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
 /* test.h and usctest.h are the two header files that are required by the
  * quickhit package.  They contain function and macro declarations which you
  * can use in your test programs
  */
#include "test.h"
#include "usctest.h"

void setup();
void cleanup();

char *TCID = "readdir02";	/* Test program identifier.    */
int TST_TOTAL = 1;		/* Total number of test cases. */
extern int Tst_count;		/* Test Case counter for tst_* routines */

int exp_enos[] = { EBADF, 0 };

/***********************************************************************
 * Main
 ***********************************************************************/
int main(int ac, char **av)
{
	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */
	DIR *test_dir;
	struct dirent *dptr;

	/* parse standard options */
	if ((msg = parse_opts(ac, av, (option_t *) NULL, NULL)) != (char *)NULL) {
		tst_brkm(TBROK, cleanup, "OPTION PARSING ERROR - %s", msg);
	}

    /***************************************************************
     * perform global setup for test
     ***************************************************************/
	setup();

	/* set the expected errnos... */
	TEST_EXP_ENOS(exp_enos);

    /***************************************************************
     * check looping state
     ***************************************************************/
	/* TEST_LOOPING() is a macro that will make sure the test continues
	 * looping according to the standard command line args.
	 */
	for (lc = 0; TEST_LOOPING(lc); lc++) {

		/* reset Tst_count in case we are looping. */
		Tst_count = 0;

		if ((test_dir = opendir(".")) == NULL) {
			tst_resm(TFAIL, "opendir(\".\") Failed, errno=%d : %s",
				 errno, strerror(errno));
		} else {
			if (closedir(test_dir) < 0) {
				tst_resm(TFAIL,
					 "closedir(\".\") Failed, errno=%d : %s",
					 errno, strerror(errno));
			} else {
				dptr = readdir(test_dir);
				switch (errno) {
				case EBADF:
					tst_resm(TPASS,
						 "expected failure - errno = %d : %s",
						 errno, strerror(errno));
					break;
				default:
					if (dptr != NULL) {
						tst_brkm(TFAIL, cleanup,
							 "call failed with an "
							 "unexpected error - %d : %s",
							 errno,
							 strerror(errno));
					} else {
						tst_resm(TINFO,
							 "readdir() is not _required_ to fail, "
							 "errno = %d  ", errno);
					}
				}
			}

		}

	}			/* End for TEST_LOOPING */

    /***************************************************************
     * cleanup and exit
     ***************************************************************/
	cleanup();

	return 0;
}				/* End main */

void sigsegv_handler(int sig)
{
	tst_resm(TCONF,
		 "This system's implementation of closedir() will not allow this test to execute properly.");
	cleanup();
}

/***************************************************************
 * setup() - performs all ONE TIME setup for this test.
 ***************************************************************/
void setup()
{

	struct sigaction act;

	/* You will want to enable some signal handling so you can capture
	 * unexpected signals like SIGSEGV.
	 */
	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	act.sa_handler = sigsegv_handler;
	(void)sigaction(SIGSEGV, &act, NULL);

	/* Pause if that option was specified */
	TEST_PAUSE;

	/* If you are doing any file work, you should use a temporary directory.  We
	 * provide tst_tmpdir() which will create a uniquely named temporary
	 * directory and cd into it.  You can now create directories in the current
	 * directory without worrying.
	 */
	tst_tmpdir();

}

/***************************************************************
 * cleanup() - performs all ONE TIME cleanup for this test at
 *		completion or premature exit.
 ***************************************************************/
void cleanup()
{
	/*
	 * print timing stats if that option was specified.
	 * print errno log if that option was specified.
	 */
	TEST_CLEANUP;

	/* If you use a temporary directory, you need to be sure you remove it. Use
	 * tst_rmdir() to do it automatically.$
	 */
	tst_rmdir();

	/* exit with return code appropriate for results */
	tst_exit();
}
