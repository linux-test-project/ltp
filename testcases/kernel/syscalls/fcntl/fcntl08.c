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
/* $Id: fcntl08.c,v 1.6 2009/10/26 14:55:46 subrata_modak Exp $ */
/**********************************************************
 *
 *    OS Test - Silicon Graphics, Inc.
 *
 *    TEST IDENTIFIER	: fcntl08
 *
 *    EXECUTED BY	: anyone
 *
 *    TEST TITLE	: Basic test for fcntl(2) using F_SETFL argument.
 *
 *    PARENT DOCUMENT	: usctpl01
 *
 *    TEST CASE TOTAL	: 1
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
 *	1.) fcntl(2) returns...(See Description)
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
 *      No run-time environmental needs.
 *
 *    SPECIAL PROCEDURAL REQUIREMENTS
 *	None
 *
 *    INTERCASE DEPENDENCIES
 *	None
 *
 *    DETAILED DESCRIPTION
 *	This is a Phase I test for the fcntl(2) system call.  It is intended
 *	to provide a limited exposure of the system call, for now.  It
 *	should/will be extended when full functional tests are written for
 *	fcntl(2).
 *
 *	Setup:
 *	  Setup signal handling.
 *	  Pause for SIGUSR1 if option specified.
 *
 *	Test:
 *	 Loop if the proper options are given.
 *	  Execute system call
 *	  Check return code, if system call failed (return=-1)
 *		Log the errno and Issue a FAIL message.
 *	  Otherwise, Issue a PASS message.
 *
 *	Cleanup:
 *        close file
 *        remove temp directory
 *
 *#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#**/

#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include "test.h"
#include "usctest.h"

void setup();
void cleanup();

char *TCID = "fcntl08";		/* Test program identifier.    */
int TST_TOTAL = 1;		/* Total number of test cases. */

char fname[255];
int arg, fd;

int main(int ac, char **av)
{
	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */

	if ((msg = parse_opts(ac, av, NULL, NULL)) != NULL)
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {

		Tst_count = 0;

		TEST(fcntl(fd, F_SETFL, arg));

		if (TEST_RETURN == -1)
			tst_resm(TFAIL|TTERRNO, "fcntl failed");
		else {

			if (STD_FUNCTIONAL_TEST) {
				tst_resm(TPASS, "fcntl returned %ld",
				    TEST_RETURN);
			}
		}

	}

	cleanup();
	tst_exit();
}

void setup()
{

	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;

	tst_tmpdir();

	sprintf(fname, "tfile_%d", getpid());
	if ((fd = open(fname, O_RDWR | O_CREAT, 0700)) == -1)
		tst_brkm(TBROK|TERRNO, cleanup, "open failed");

	/* set the flags to be used */
#ifdef CRAY
	arg = (O_NDELAY | O_APPEND | O_RAW | O_NONBLOCK);
#else /* ! CRAY */
#ifdef __hpux
	/* HP-UX doesn't allow O_NDELAY and O_NONBLOCK;
	   we choose O_NONBLOCK (POSIX style) */
	arg = (O_APPEND | O_NONBLOCK);
#else /* ! __hpux */
	arg = (O_NDELAY | O_APPEND | O_NONBLOCK);
#endif /* ! __hpux */
#endif /* ! CRAY */

}

void cleanup()
{
	TEST_CLEANUP;

	if (close(fd) == -1)
		tst_resm(TWARN|TERRNO, "close failed");

	tst_rmdir();

}