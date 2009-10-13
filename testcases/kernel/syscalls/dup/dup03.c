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
/* $Id: dup03.c,v 1.5 2009/10/13 14:00:46 subrata_modak Exp $ */
/**********************************************************
 *
 *    OS Test - Silicon Graphics, Inc.
 *
 *    TEST IDENTIFIER	: dup03
 *
 *    EXECUTED BY	: anyone
 *
 *    TEST TITLE	: Negative test for dup(2) (too many fds)
 *
 *    PARENT DOCUMENT	: usctpl01
 *
 *    TEST CASE TOTAL	: 1
 *
 *    WALL CLOCK TIME	: 1
 *
 *    CPU TYPES		: ALL
 *
 *    AUTHOR		: Richard Logan
 *
 *    CO-PILOT		: William Roske
 *
 *    DATE STARTED	: 06/94
 *
 *    INITIAL RELEASE	: UNICOS 7.0
 *
 *    TEST CASES
 *
 * 	1.) dup(2) returns...(See Description)
 *
 *    INPUT SPECIFICATIONS
 * 	The standard options for system call tests are accepted.
 *	(See the parse_opts(3) man page).
 *
 *    OUTPUT SPECIFICATIONS
 *$
 *    DURATION
 * 	Terminates - with frequency and infinite modes.
 *
 *    SIGNALS
 * 	Uses SIGUSR1 to pause before test if option set.
 * 	(See the parse_opts(3) man page).
 *
 *    RESOURCES
 * 	None
 *
 *    ENVIRONMENTAL NEEDS
 *      No run-time environmental needs.
 *
 *    SPECIAL PROCEDURAL REQUIREMENTS
 * 	None
 *
 *    INTERCASE DEPENDENCIES
 * 	None
 *
 *    DETAILED DESCRIPTION
 *
 * 	Setup:
 * 	  Setup signal handling.
 *	  Pause for SIGUSR1 if option specified.
 *
 * 	Test:
 *	 Loop if the proper options are given.
 * 	  Execute system call
 *	  Check return code, if system call failed (return=-1)
 *		Log the errno and Issue a FAIL message.
 *	  Otherwise, Issue a PASS message.
 *
 * 	Cleanup:
 * 	  Print errno log and/or timing stats if options given
 *
 *
 *#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#**/

#include <sys/types.h>
#include <sys/fcntl.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <stdlib.h>
#include "test.h"
#include "usctest.h"

void setup();
void cleanup();

char *TCID = "dup03";		/* Test program identifier.    */
int TST_TOTAL = 1;		/* Total number of test cases. */
extern int Tst_count;		/* Test Case counter for tst_* routines */

char Fname[255];
int *Fd = NULL;
int Nfds = 0;

/***********************************************************************
 * Main
 ***********************************************************************/
int main(int ac, char **av)
{
	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */

    /***************************************************************
     * parse standard options
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
		 * Call dup(2)
		 */
		TEST(dup(Fd[0]));

		/* check return code */
		if (TEST_RETURN == -1) {
			if (STD_FUNCTIONAL_TEST) {
				if (TEST_ERRNO == EMFILE) {
					tst_resm(TPASS,
						 "dup(%d) Failed, errno=%d : %s",
						 Fd[0], TEST_ERRNO,
						 strerror(TEST_ERRNO));
				} else {
					tst_resm(TFAIL,
						 "dup(%d) Failed, errno=%d %s, expected %d (EMFILE)",
						 Fd[0], TEST_ERRNO,
						 strerror(TEST_ERRNO), EMFILE);
				}
			}
		} else {
			tst_resm(TFAIL,
				 "dup(%d) returned %ld, expected -1, errno:%d (EMFILE)",
				 Fd[0], TEST_RETURN, EMFILE);

			/* close the new file so loops do not open too many files */
			if (close(TEST_RETURN) == -1) {
				tst_brkm(TBROK, cleanup,
					 "close(%s) Failed, errno=%d : %s",
					 Fname, errno, strerror(errno));
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
	long maxfds;

	/*
	 * Initialize Fd in case we get a quick signal
	 */
	maxfds = sysconf(_SC_OPEN_MAX);
	if (maxfds < 1) {
		tst_brkm(TBROK, cleanup,
			 "sysconf(_SC_OPEN_MAX) Failed, errno=%d : %s",
			 errno, strerror(errno));
	}

	Fd = (int *)malloc(maxfds * sizeof(int));
	Fd[0] = -1;

	/* capture signals */
	tst_sig(FORK, DEF_HANDLER, cleanup);

	/* Pause if that option was specified */
	TEST_PAUSE;

	/* make a temp directory and cd to it */
	tst_tmpdir();

	/*
	 * open the file as many times as it takes to use up all fds
	 */
	sprintf(Fname, "dupfile");
	for (Nfds = 1; Nfds <= maxfds; Nfds++) {
		if ((Fd[Nfds - 1] = open(Fname, O_RDWR | O_CREAT, 0700)) == -1) {

			Nfds--;	/* on a open failure, decrement the counter */
			if (errno == EMFILE) {
				break;
			} else {	/* open failed for some other reason */
				tst_brkm(TBROK, cleanup,
					 "open(%s, O_RDWR|O_CREAT,0700) Failed, errno=%d : %s",
					 Fname, errno, strerror(errno));
			}
		}
	}

	/*
	 * make sure at least one was open and that all fds were opened.
	 */
	if (Nfds == 0) {
		tst_brkm(TBROK, cleanup, "Unable to open at least one file");
	}
	if (Nfds > maxfds) {
		tst_brkm(TBROK, cleanup,
			 "Unable to open enough files to use all file descriptors, tried %ld",
			 maxfds);
	}
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

	/* close the open file we've been dup'ing */
	if (Fd) {
		for (; Nfds > 0; Nfds--) {
			if (close(Fd[Nfds - 1]) == -1) {
				tst_resm(TWARN,
					 "close(%s) Failed, errno=%d : %s",
					 Fname, errno, strerror(errno));
			}
			Fd[Nfds] = -1;
		}
		free(Fd);
	}

	/* Remove tmp dir and all files in it */
	tst_rmdir();

	/* exit with return code appropriate for results */
	tst_exit();
}				/* End cleanup() */
