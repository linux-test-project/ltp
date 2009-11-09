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
/* $Id: access01.c,v 1.8 2009/11/09 05:56:58 yaberauneya Exp $ */
/**********************************************************
 *
 *    OS Test - Silicon Graphics, Inc.
 *
 *    TEST IDENTIFIER	: access01
 *
 *    EXECUTED BY	: anyone
 *
 *    TEST TITLE	: Basic test for access(2) using F_OK,
 *                        R_OK, W_OK and X_OK arguments.
 *
 *    PARENT DOCUMENT	: usctpl01
 *
 *    TEST CASE TOTAL	: 6
 *
 *    WALL CLOCK TIME	: 1
 *
 *    CPU TYPES		: ALL
 *
 *    AUTHOR		: William Roske
 *
 *    CO-PILOT		: Dave Fenner
 *
 *    DATE STARTED	: 03/30/92
 *
 *    INITIAL RELEASE	: UNICOS 7.0
 *
 *    TEST CASES
 *
 *	1.) access(2) returns 0 for F_OK...(See Description)
 *	2.) access(2) returns 0 for R_OK...(See Description)
 *	3.) access(2) returns 0 for W_OK...(See Description)
 *	4.) access(2) returns 0 for X_OK...(See Description)
 *
 *    INPUT SPECIFICATIONS
 *	The standard options for system call tests are accepted.
 *	(See the parse_opts(3) man page).
 *
 *    OUTPUT SPECIFICATIONS
 *
 *    DURATION
 *	Terminates - with frequency and infinite modes.
 *
 *    SIGNALS
 *	Uses SIGUSR1 to pause before test if option set.
 *	(See the parse_opts(3) man page).
 *
 *    RESOURCES
 *	None
 *
 *    ENVIRONMENTAL NEEDS
 *	The libcuts.a and libsys.a libraries must be included in
 *	the compilation of this test.
 *
 *    SPECIAL PROCEDURAL REQUIREMENTS
 *	None
 *
 *    INTERCASE DEPENDENCIES
 *	None
 *
 *    DETAILED DESCRIPTION
 *	This is a Phase I test for the access(2) system call.  It is intended
 *	to provide a limited exposure of the system call, for now.  It
 *	should/will be extended when full functional tests are written for
 *	access(2).
 *
 *	Setup:
 *	  Setup signal handling.
 *	  Pause for SIGUSR1 if option specified.
 *	  Create a temp directory and cd to it.
 *	  Creat a temp file wil read, write and execute permissions.
 *
 *	Test:
 *	 Loop if the proper options are given.
 *	  Execute system call with F_OK on tmp file
 *	  Check return code, if system call failed (return=-1)
 *		Log the errno and Issue a FAIL message.
 *	  Otherwise, Issue a PASS message.
 *	  Execute system call with X_OK on tmp file...
 *	  Execute system call with W_OK on tmp file...
 *	  Execute system call with R_OK on tmp file...
 *
 *	Cleanup:
 *	  Print errno log
 *
 *
 *#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#**/

#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/fcntl.h>
#include <errno.h>
#include <signal.h>
#include <sys/stat.h>
#include "test.h"
#include "usctest.h"
void setup();
void cleanup();

char *TCID = "access01";	/* Test program identifier.    */
int TST_TOTAL = 4;		/* Total number of test cases. */
extern int Tst_count;		/* Test Case counter for tst_* routines */

char Fname[255];

static struct test_case_t {
	char *file;
	int mode;
	char *string;
	int experrno;
} Test_cases[] = {
	{ Fname, F_OK, "F_OK", 0},
	{ Fname, X_OK, "X_OK", 0},
	{ Fname, W_OK, "W_OK", 0},
	{ Fname, R_OK, "R_OK", 0},
};

int Ntc = sizeof(Test_cases) / sizeof(struct test_case_t);

/***********************************************************************
 * Main
 ***********************************************************************/
int main(int ac, char **av)
{
	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */
	int tc;

	TST_TOTAL = Ntc;

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

		for (tc = 0; tc < Ntc; tc++) {
			/*
			 * Call access(2)
			 */
			TEST(access(Test_cases[tc].file, Test_cases[tc].mode));

			/* check return code */
			if (TEST_RETURN == -1 && Test_cases[tc].experrno == 0) {
				tst_resm(TFAIL|TTERRNO,
					 "access(%s, %s) failed",
					 Test_cases[tc].file,
					 Test_cases[tc].string);

			} else if (TEST_RETURN != -1
				   && Test_cases[tc].experrno != 0) {
				tst_resm(TFAIL,
					 "access(%s, %s) returned %ld, exp -1, errno:%d",
					 Test_cases[tc].file,
					 Test_cases[tc].string, TEST_RETURN,
					 Test_cases[tc].experrno);
			} else {

		/***************************************************************
	         * only perform functional verification if flag set (-f not given)
	         ***************************************************************/
				if (STD_FUNCTIONAL_TEST) {
					/* No Verification test, yet... */
					tst_resm(TPASS,
						 "access(%s, %s) returned %ld",
						 Test_cases[tc].file,
						 Test_cases[tc].string,
						 TEST_RETURN);
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

/***************************************************************
 * setup() - performs all ONE TIME setup for this test.
 ***************************************************************/
void setup()
{
	int fd;
	struct stat stbuf;

	/* capture signals */
	tst_sig(FORK, DEF_HANDLER, cleanup);

	umask(0);		/* reset umask avoid it affects on modes */

	/* Pause if that option was specified */
	TEST_PAUSE;

	/* make a temp directory and cd to it */
	tst_tmpdir();

	/*
	 * Since files inherit group ids, make sure our dir has a valid grp
	 * to us.
	 */
	if (chown(".", -1, getegid()) < 0) {
		tst_brkm(TBROK|TERRNO, cleanup,
			 "chown(\".\", -1, %d) failed", getegid());
	}

	sprintf(Fname, "accessfile");

	fd = open(Fname, O_RDWR | O_CREAT, 06777);
	if (fd == -1)
		tst_brkm(TBROK|TERRNO, cleanup,
			"open(%s, O_RDWR|O_CREAT, 06777) failed", Fname);
	else if (close(fd) == -1)
		tst_resm(TINFO|TERRNO, "close(%s) failed", Fname);

	/*
	 * force the mode to be set to 6777
	 */
	if (chmod(Fname, 06777) == -1)
		tst_brkm(TBROK|TERRNO, cleanup, "chmod(%s, 06777) failed", Fname);

	stat(Fname, &stbuf);

	if ((stbuf.st_mode & 06777) != 06777) {
		/*
		 * file can not be properly setup
		 */
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
	 */
	TEST_CLEANUP;

	/* remove the temp dir */
	tst_rmdir();

	/* exit with return code appropriate for results */
	tst_exit();

}				/* End cleanup() */
