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
/* $Id: setregid01.c,v 1.6 2009/11/02 13:57:18 subrata_modak Exp $ */
/**********************************************************
 *
 *    OS Test - Silicon Graphics, Inc.
 *
 *    TEST IDENTIFIER	: setregid01
 *
 *    EXECUTED BY	: anyone
 *
 *    TEST TITLE	: Basic test for setregid(2)
 *
 *    PARENT DOCUMENT	: usctpl01
 *
 *    TEST CASE TOTAL	: 5
 *
 *    WALL CLOCK TIME	: 1
 *
 *    CPU TYPES		: ALL
 *
 *    AUTHOR		: William Roske
 *
 *    CO-PILOT		: Dave Fenner
 *
 *    DATE STARTED	: 05/13/92
 *
 *    INITIAL RELEASE	: UNICOS 7.0
 *
 *    TEST CASES
 *
 * 	1.) setregid(2) returns...(See Description)
 *
 *    INPUT SPECIFICATIONS
 * 	The standard options for system call tests are accepted.
 *	(See the parse_opts(3) man page).
 *
 *    OUTPUT SPECIFICATIONS
 *
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
 *	This is a Phase I test for the setregid(2) system call.  It is intended
 *	to provide a limited exposure of the system call, for now.  It
 *	should/will be extended when full functional tests are written for
 *	setregid(2).
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

#include <sys/types.h>

#include "test.h"
#include "usctest.h"

void setup();
void cleanup();

char *TCID = "setregid01";	/* Test program identifier.    */
int TST_TOTAL = 5;		/* Total number of test cases. */

int exp_enos[] = { 0, 0 };	/* Zero terminated list of expected errnos */

int gid, egid;			/* current real and effective group id */

int main(int ac, char **av)
{
	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */

	if ((msg = parse_opts(ac, av, NULL, NULL)) != NULL)
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);

	setup();

	/* set the expected errnos... */
	TEST_EXP_ENOS(exp_enos);

	for (lc = 0; TEST_LOOPING(lc); lc++) {

		Tst_count = 0;

		/*
		 * TEST CASE:
		 *  Dont change either real or effective gid
		 */
		gid = getgid();
		egid = getegid();

		/* Call setregid(2) */
		TEST(setregid(-1, -1));

		/* check return code */
		if (TEST_RETURN == -1) {
			TEST_ERROR_LOG(TEST_ERRNO);
			tst_resm(TFAIL,
				 "setregid -  Dont change either real or effective gid failed, errno=%d : %s",
				 TEST_ERRNO, strerror(TEST_ERRNO));
		} else {
			if (STD_FUNCTIONAL_TEST) {
				/* No Verification test, yet... */
				tst_resm(TPASS,
					 "setregid -  Dont change either real or effective gid returned %ld",
					 TEST_RETURN);
			}
		}

		/*
		 * TEST CASE:
		 *  change effective to effective gid
		 */

		/* Call setregid(2) */
		TEST(setregid(-1, egid));

		/* check return code */
		if (TEST_RETURN == -1) {
			TEST_ERROR_LOG(TEST_ERRNO);
			tst_resm(TFAIL,
				 "setregid -  change effective to effective gid failed, errno=%d : %s",
				 TEST_ERRNO, strerror(TEST_ERRNO));
		} else {
			if (STD_FUNCTIONAL_TEST) {
				/* No Verification test, yet... */
				tst_resm(TPASS,
					 "setregid -  change effective to effective gid returned %ld",
					 TEST_RETURN);
			}
		}

		/*
		 * TEST CASE:
		 *  change real to real gid
		 */

		/* Call setregid(2) */
		TEST(setregid(gid, -1));

		/* check return code */
		if (TEST_RETURN == -1) {
			TEST_ERROR_LOG(TEST_ERRNO);
			tst_resm(TFAIL,
				 "setregid -  change real to real gid failed, errno=%d : %s",
				 TEST_ERRNO, strerror(TEST_ERRNO));
		} else {
			if (STD_FUNCTIONAL_TEST) {
				/* No Verification test, yet... */
				tst_resm(TPASS,
					 "setregid -  change real to real gid returned %ld",
					 TEST_RETURN);
			}
		}

		/*
		 * TEST CASE:
		 *  change effective to real gid
		 */

		/* Call setregid(2) */
		TEST(setregid(-1, gid));

		/* check return code */
		if (TEST_RETURN == -1) {
			TEST_ERROR_LOG(TEST_ERRNO);
			tst_resm(TFAIL,
				 "setregid -  change effective to real gid failed, errno=%d : %s",
				 TEST_ERRNO, strerror(TEST_ERRNO));
		} else {
			if (STD_FUNCTIONAL_TEST) {
				/* No Verification test, yet... */
				tst_resm(TPASS,
					 "setregid -  change effective to real gid returned %ld",
					 TEST_RETURN);
			}
		}

		/*
		 * TEST CASE:
		 *  try to change real to current real
		 */

		/* Call setregid(2) */
		TEST(setregid(gid, gid));

		/* check return code */
		if (TEST_RETURN == -1) {
			TEST_ERROR_LOG(TEST_ERRNO);
			tst_resm(TFAIL|TTERRNO, "setregid failed");
		} else {
			if (STD_FUNCTIONAL_TEST) {
				/* No Verification test, yet... */
				tst_resm(TPASS, "setregid return %ld",
				    TEST_RETURN);
			}
		}

	}

	cleanup();
	tst_exit();
	tst_exit();

}

/***************************************************************
 * setup() - performs all ONE TIME setup for this test.
 ***************************************************************/
void setup()
{

	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;

	/* make a temp dir and cd to it */
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

	tst_rmdir();
}
