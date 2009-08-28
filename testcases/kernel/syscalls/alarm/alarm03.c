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
 */
/* $Id: alarm03.c,v 1.10 2009/08/28 10:57:29 vapier Exp $ */
/**********************************************************
 *
 *    OS Test - Silicon Graphics, Inc.
 *
 *    TEST IDENTIFIER	: alarm03
 *
 *    EXECUTED BY	: anyone
 *
 *    TEST TITLE	: alarm(2) cleared by a fork
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
 *    CO-PILOT		: Dennis Arason
 *
 *    DATE STARTED	: 08/96
 *
 *
 *    TEST CASES
 *
 * 	1.) alarm(100), fork, child's alarm(0) shall return 0;
 *	2.) alarm(100), fork, parent's alarm(0) shall return non-zero.
 *
 *    INPUT SPECIFICATIONS
 * 	The standard options for system call tests are accepted.
 *	(See the parse_opts(3) man page).
 *
 *
 *    SPECIAL PROCEDURAL REQUIREMENTS
 * 	None
 *
 *
 *    DETAILED DESCRIPTION
 *	This is a Phase I test for the alarm(2) system call.  It is intended
 *	to provide a limited exposure of the system call, for now.  It
 *	should/will be extended when full functional tests are written for
 *	alarm(2).
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

#include <errno.h>
#include <string.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/wait.h>
#include "test.h"
#include "usctest.h"

void setup();
void cleanup();

char *TCID = "alarm03";		/* Test program identifier.    */
int TST_TOTAL = 1;		/* Total number of test cases. */
extern int Tst_count;		/* Test Case counter for tst_* routines */

int main(int ac, char **av)
{
	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */
	int e_code, status, retval = 0;

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
		 * Call alarm(2)
		 */
		TEST(alarm(100));

		switch (FORK_OR_VFORK()) {
		case -1:
			tst_brkm(TBROK|TERRNO, cleanup, "fork() failed");
			break;

		case 0:
			TEST(alarm(0));

			if (TEST_RETURN != 0) {
				retval = 1;
				tst_resm(TFAIL,
					 "alarm(100), fork, alarm(0) child's alarm returned %ld",
					 TEST_RETURN);
			} else if (STD_FUNCTIONAL_TEST) {
				tst_resm(TPASS,
					 "alarm(100), fork, alarm(0) child's alarm returned %ld",
					 TEST_RETURN);
			}

			exit(retval);
			break;

		default:
			Tst_count++;
			TEST(alarm(0));
/* The timer may be rounded up to the next nearest second, this is OK */
			if (TEST_RETURN <= 0 || TEST_RETURN > 101) {
				retval = 1;
				tst_resm(TFAIL,
					 "alarm(100), fork, alarm(0) parent's alarm returned %ld",
					 TEST_RETURN);
			} else if (STD_FUNCTIONAL_TEST) {
				tst_resm(TPASS,
					 "alarm(100), fork, alarm(0) parent's alarm returned %ld",
					 TEST_RETURN);
			}
			/* wait for the child to finish */
			wait(&status);
			/* make sure the child returned a good exit status */
			e_code = status >> 8;
			if ((e_code != 0) || (retval != 0)) {
				tst_resm(TFAIL, "Failures reported above");
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
	void trapper();

	/* capture signals */
	tst_sig(FORK, DEF_HANDLER, cleanup);

	signal(SIGALRM, trapper);

	/* Pause if that option was specified */
	TEST_PAUSE;
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

	/* exit with return code appropriate for results */
	tst_exit();
}				/* End cleanup() */

void trapper(sig)
int sig;
{
	signal(SIGALRM, trapper);
}
