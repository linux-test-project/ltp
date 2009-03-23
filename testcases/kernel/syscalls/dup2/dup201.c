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
 *	dup201.c
 *
 * DESCRIPTION
 *	Negative tests for dup2() with bad fd (EBADF)
 *
 * ALGORITHM
 * 	Setup:
 *	a.	Setup signal handling.
 *	b.	Pause for SIGUSR1 if option specified.
 *
 * 	Test:
 *	a.	Loop if the proper options are given.
 *	b.	Loop through the test cases
 * 	c.	Execute dup2() system call
 *	d.	Check return code, if system call failed (return=-1), test
 *		for EBADF.
 *
 * 	Cleanup:
 * 	a.	Print errno log and/or timing stats if options given
 *
 * USAGE:  <for command-line>
 *  dup201 [-c n] [-e] [-i n] [-I x] [-P x] [-t]
 *     where,  -c n : Run n copies concurrently.
 *             -e   : Turn on errno logging.
 *             -i n : Execute test n times.
 *             -I x : Execute test for x seconds.
 *             -P x : Pause for x seconds between iterations.
 *             -t   : Turn on syscall timing.
 *
 * HISTORY
 *	07/2001 Ported by Wayne Boyer
 *	01/2002 Removed EMFILE test - Paul Larson
 *
 * RESTRICTIONS
 * 	NONE
 */

#include <sys/types.h>
#include <sys/fcntl.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <unistd.h>
#include <signal.h>
#include <test.h>
#include <usctest.h>

void setup(void);
void cleanup(void);

char *TCID = "dup201";		/* Test program identifier.    */
int TST_TOTAL = 4;		/* Total number of test cases. */
extern int Tst_count;		/* Test counter for tst_* routines */

int maxfd;
int goodfd = 5;
int badfd = -1;
int mystdout = 0;
int fd, fd1;
int mypid;
char fname[20];

int exp_enos[] = { EBADF, 0 };

struct test_case_t {
	int *ofd;
	int *nfd;
	int error;
	void (*setupfunc) ();
} TC[] = {
	/* First fd argument is less than 0 - EBADF */
	{
	&badfd, &goodfd, EBADF, NULL},
	    /* First fd argument is getdtablesize() - EBADF */
	{
	&maxfd, &goodfd, EBADF, NULL},
	    /* Second fd argument is less than 0 - EBADF */
	{
	&mystdout, &badfd, EBADF, NULL},
	    /* Second fd argument is getdtablesize() - EBADF */
	{
&mystdout, &maxfd, EBADF, NULL},};

int main(int ac, char **av)
{
	int lc;			/* loop counter */
	int i, j;
	char *msg;		/* message returned from parse_opts */

	/* parse standard options */
	if ((msg = parse_opts(ac, av, (option_t *) NULL, NULL)) != (char *)NULL) {
		tst_brkm(TBROK, cleanup, "OPTION PARSING ERROR - %s", msg);
	}

	setup();

	/* set up the expected errnos */
	TEST_EXP_ENOS(exp_enos);

	/* check looping state if -i option given */
	for (lc = 0; TEST_LOOPING(lc); lc++) {
		/* reset Tst_count in case we are looping. */
		Tst_count = 0;

		/* loop through the test cases */

		for (i = 0; i < TST_TOTAL; i++) {

			/* call the test case setup routine if necessary */
			if (TC[i].setupfunc != NULL) {
				(*TC[i].setupfunc) ();
			}

			TEST(dup2(*TC[i].ofd, *TC[i].nfd));

			if (TEST_RETURN != -1) {
				tst_resm(TFAIL, "call succeeded unexpectedly");
				continue;
			}

			TEST_ERROR_LOG(TEST_ERRNO);

			if (TEST_ERRNO == TC[i].error) {
				tst_resm(TPASS, "expected failure - "
					 "errno = %d : %s", TEST_ERRNO,
					 strerror(TEST_ERRNO));
			} else {
				tst_resm(TFAIL, "unexpected error - %d : %s - "
					 "expected %d", TEST_ERRNO,
					 strerror(TEST_ERRNO), TC[i].error);
			}
		}
		/* cleanup things in case we are looping */
		for (j = fd1; j < maxfd; j++) {
			sprintf(fname, "dup201.%d.%d", j, mypid);
			(void)close(j);
			(void)unlink(fname);
		}
	}
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

	/* make a temp directory and cd to it */
	tst_tmpdir();

	/* get some test specific values */
	maxfd = getdtablesize();
	mypid = getpid();
}

/*
 * cleanup() - performs all ONE TIME cleanup for this test at
 *	       completion or premature exit.
 */
void cleanup()
{
	/*
	 * print timing stats if that option was specified.
	 * print errno log if that option was specified.
	 */
	TEST_CLEANUP;

	/* Remove tmp dir and all files in it */
	tst_rmdir();

	/* exit with return code appropriate for results */
	tst_exit();
}
