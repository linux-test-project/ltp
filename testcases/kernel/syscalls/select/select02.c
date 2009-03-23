/*
 * Copyright (c) 2000 Silicon Graphics, Inc.  All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it would be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * Further, this software is distributed without any warranty that it is
 * free of the rightful claim of any third person regarding infringement
 * or the like.  Any license provided herein, whether implied or
 * otherwise, applies only to this software file.  Patent licenses, if
 * any, provided herein do not apply to combinations of this program with
 * other software, or any other product whatsoever.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write the Free Software Foundation, Inc., 59
 * Temple Place - Suite 330, Boston MA 02111-1307, USA.
 *
 * Contact information: Silicon Graphics, Inc., 1600 Amphitheatre Pkwy,
 * Mountain View, CA  94043, or:
 *
 * http://www.sgi.com
 *
 * For further information regarding this notice, see:
 *
 * http://oss.sgi.com/projects/GenInfo/NoticeExplan/
 *
 */
/* $Id: select02.c,v 1.4 2009/03/23 13:36:02 subrata_modak Exp $ */
/**********************************************************
 *
 *    OS Test - Silicon Graphics, Inc.
 *
 *    TEST IDENTIFIER   : select02
 *
 *    EXECUTED BY       : anyone
 *
 *    TEST TITLE        : select of system pipe fds
 *
 *    PARENT DOCUMENT   : usctpl01
 *
 *    TEST CASE TOTAL   : 1
 *
 *    WALL CLOCK TIME   : 1
 *
 *    CPU TYPES         : ALL
 *
 *    AUTHOR            : Richard Logan
 *
 *    CO-PILOT          : Glen Overby
 *
 *    DATE STARTED      : 02/24/93
 *
 *    INITIAL RELEASE   : UNICOS 7.0
 *
 *    TEST CASES
 *
 *      1.) select(2) to fd of system pipe with no I/O and small timeout
 *
 *    INPUT SPECIFICATIONS
 *      The standard options for system call tests are accepted.
 *      (See the parse_opts(3) man page).
 *
 *    OUTPUT SPECIFICATIONS
 *
 *    DURATION
 *      Terminates - with frequency and infinite modes.
 *
 *    SIGNALS
 *      Uses SIGUSR1 to pause before test if option set.
 *      (See the parse_opts(3) man page).
 *
 *    RESOURCES
 *      None
 *
 *    ENVIRONMENTAL NEEDS
 *      No run-time environmental needs.
 *
 *    SPECIAL PROCEDURAL REQUIREMENTS
 *      None
 *
 *    INTERCASE DEPENDENCIES
 *      None
 *
 *    DETAILED DESCRIPTION
 *      This is a Phase I test for the select(2) system call.  It is intended
 *      to provide a limited exposure of the system call, for now.
 *
 *      Setup:
 *        Setup signal handling.
 *        Pause for SIGUSR1 if option specified.
 *
 *      Test:
 *       Loop if the proper options are given.
 *        Execute system call
 *        Check return code, if system call failed (return=-1)
 *              Log the errno and Issue a FAIL message.
 *        Otherwise, Issue a PASS message.
 *
 *      Cleanup:
 *        Print errno log and/or timing stats if options given
 *
 *#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#**/

#include <errno.h>
#include <signal.h>
#include <fcntl.h>		/* For open system call parameters.  */
#include <signal.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/time.h>

#include "test.h"
#include "usctest.h"

void setup();
void cleanup();

char *TCID = "select02";	/* Test program identifier.    */
int TST_TOTAL = 1;		/* Total number of test cases. */
extern int Tst_count;		/* Test Case counter for tst_* routines */

int Fd[2];
fd_set saved_Readfds, saved_Writefds;
fd_set Readfds, Writefds;

/***********************************************************************
 * MAIN
 ***********************************************************************/
int main(int ac, char **av)
{
	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */
	struct timeval timeout;
	long test_time = 0;	/* in usecs */

    /***************************************************************
     * parse standard options, and exit if there is an error
     ***************************************************************/
	if ((msg = parse_opts(ac, av, (option_t *) NULL, NULL)) != (char *)NULL) {
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);
		tst_exit();
	}

    /***************************************************************
     * perform global setup for test
     ***************************************************************/
	setup();

    /***************************************************************
     * check looping state if -c option given
     ***************************************************************/
	for (lc = 0; TEST_LOOPING(lc); lc++) {

		/* reset Tst_count in case we are looping. */
		Tst_count = 0;

		/*
		 * Assigning the specified seconds within the timeval structure.
		 */

		test_time = ((lc % 2000) * 100000);	/* 100 milli-seconds */

		/*
		 * Bound the time to a value less than 60 seconds
		 */

		if (test_time > 1000000 * 60)
			test_time = test_time % (1000000 * 60);

		timeout.tv_sec = test_time / 1000000;
		timeout.tv_usec = test_time - (timeout.tv_sec * 1000000);

		Readfds = saved_Readfds;
		Writefds = saved_Writefds;

		/* Call the system call being tested. */

		TEST(select(5, &Readfds, &Writefds, 0, &timeout));

		/* check return code */
		if (TEST_RETURN == -1) {
			TEST_ERROR_LOG(TEST_ERRNO);
			tst_resm(TFAIL,
				 "%d select(5, &Readfds, &Writefds, 0, &timeout) failed, errno=%d\n",
				 lc, errno);
		} else {

	    /***************************************************************
    	     * only perform functional verification if flag set (-f not given)
    	     ***************************************************************/
			if (STD_FUNCTIONAL_TEST) {
				/* Perform functional verification here */
				tst_resm(TPASS,
					 "select(5, &Readfds, &Writefds, 0, &timeout) timeout = %ld usecs",
					 test_time);
			}
		}

	}			/* End for TEST_LOOPING */

    /***************************************************************
     * cleanup and exit
     ***************************************************************/
	cleanup();

	return 0;
}				/* End main */

/***************************************************************
 * setup() - performs all ONE TIME setup for this test.
 ***************************************************************/
void setup()
{
	/* capture signals */
	tst_sig(FORK, DEF_HANDLER, cleanup);

	/* Pause if that option was specified */
	TEST_PAUSE;

	/* create a temporary directory and go to it */
	tst_tmpdir();

	if (pipe(Fd) == -1) {
		tst_brkm(TBROK, cleanup, "pipe(&Fd) failed, errno=%d", errno);
	}

	/*
	 * Initializing and assigning the standard output file descriptor to
	 * fd_set for select.
	 */

	FD_ZERO(&saved_Readfds);
	FD_ZERO(&saved_Writefds);
	FD_SET(Fd[0], &saved_Readfds);
	FD_SET(Fd[1], &saved_Writefds);

}				/* End setup() */

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

	/* remove temporary directory and all files in it. */
	tst_rmdir();

	/* exit with return code appropriate for results */
	tst_exit();
}				/* End cleanup() */
