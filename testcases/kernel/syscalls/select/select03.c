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
/* $Id: select03.c,v 1.6 2009/03/23 13:36:02 subrata_modak Exp $ */
/**********************************************************
 *
 *    OS Test - Silicon Graphics, Inc.
 *
 *    TEST IDENTIFIER   : select03
 *
 *    EXECUTED BY       : anyone
 *
 *    TEST TITLE        : select of fd of a named-pipe (FIFO)
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
 *      1.) select(2) of fd of a named-pipe (FIFO) with no I/O and small timeout value
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
 *
 *#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#**/

#include <errno.h>
#include <signal.h>
#include <fcntl.h>		/* For open system call parameters.  */
#include <signal.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/stat.h>

#include "test.h"
#include "usctest.h"

#define FILENAME	"select03"

void setup();
void cleanup();

char *TCID = "select03";	/* Test program identifier.    */
int TST_TOTAL = 1;		/* Total number of test cases. */

int Fd;
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
	if ((msg = parse_opts(ac, av, NULL, NULL)) != NULL) {
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);

	}

    /***************************************************************
     * perform global setup for test
     ***************************************************************/
	setup();

    /***************************************************************
     * check looping state if -c option given
     ***************************************************************/
	for (lc = 0; TEST_LOOPING(lc); lc++) {

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
				 "%d select(5, &Readfds, &Writefds, 0, &timeout) failed errno=%d\n",
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

	}

    /***************************************************************
     * cleanup and exit
     ***************************************************************/
	cleanup();
	tst_exit();

}

/***************************************************************
 * setup() - performs all ONE TIME setup for this test.
 ***************************************************************/
void setup()
{

	tst_sig(FORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;

	/* create a temporary directory and go to it */
	tst_tmpdir();

	/* make and open FIFO */
	if (mkfifo(FILENAME, 0777) == -1) {
		tst_brkm(TBROK, cleanup, "mkfifo(%s, 0777) failed, errno=%d",
			 FILENAME, errno);
	}

	if ((Fd = open(FILENAME, O_RDWR)) == -1) {
		tst_brkm(TBROK, cleanup, "open(%s, O_RDWR) failed, errno=%d",
			 FILENAME, errno);
	}

	/*
	 * Initializing and assigning the standard output file descriptor to
	 * fd_set for select.
	 */

	FD_ZERO(&saved_Readfds);
	FD_ZERO(&saved_Writefds);
	FD_SET(Fd, &saved_Readfds);
	FD_SET(Fd, &saved_Writefds);

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
	close(Fd);

	TEST_CLEANUP;

	tst_rmdir();

}
