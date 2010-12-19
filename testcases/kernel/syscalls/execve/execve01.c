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
/* $Id: execve01.c,v 1.6 2009/03/23 13:35:40 subrata_modak Exp $ */
/**********************************************************
 *
 *    OS Test - Silicon Graphics, Inc.
 *
 *    TEST IDENTIFIER	: execve01
 *
 *    EXECUTED BY	: anyone
 *
 *    TEST TITLE	: Basic test for execve(2)
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
 *    DATE STARTED	: 06/01/02
 *
 *    INITIAL RELEASE	: UNICOS 7.0
 *
 *    TEST CASES
 *
 * 	1.) execve(2) returns...(See Description)
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
 *	This is a Phase I test for the execve(2) system call.  It is intended
 *	to provide a limited exposure of the system call, for now.  It
 *	should/will be extended when full functional tests are written for
 *	execve(2).
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

#include <sys/types.h>
#include <sys/wait.h>

#include "test.h"
#include "usctest.h"

void setup();
void cleanup();

char *TCID = "execve01";	/* Test program identifier.    */
int TST_TOTAL = 1;		/* Total number of test cases. */

int exp_enos[] = { 0, 0 };	/* Zero terminated list of expected errnos */

pid_t pid;			/* process id from fork */
int status;			/* status returned from waitpid */
char *args[2] = { "/usr/bin/test", 0 };	/* argument list for execve call */

int main(int ac, char **av, char **environ)
{
	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */


	if ((msg = parse_opts(ac, av, NULL, NULL)) != NULL)
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);

	setup();

	TEST_EXP_ENOS(exp_enos);

	for (lc = 0; TEST_LOOPING(lc); lc++) {

		Tst_count = 0;

		switch (pid = FORK_OR_VFORK()) {
		case 0:	/* CHILD - Call execve(2) */
			execve("/usr/bin/test", args, environ);
			/* should not get here!! if we do, the parent will fail the Test Case */
			exit(errno);
		case -1:	/* ERROR!!! exit now!! */
			tst_brkm(TBROK, cleanup, "fork failed");
			break;
		default:
			waitpid(pid, &status, 0);
			if (WIFEXITED(status)) {
				if (STD_FUNCTIONAL_TEST) {
					tst_resm(TPASS,
						 "execve - properly exec's a simple program..");
				}
			} else {
				TEST_ERROR_LOG(WEXITSTATUS(status));
				tst_resm(TFAIL,
					 "Child process did not terminate properly, status=%d",
					 status);
			}
			break;
		}		/* switch */

	}

	cleanup();

	tst_exit();
}

void setup()
{

	tst_sig(FORK, DEF_HANDLER, cleanup);

	if (STD_TIMING_ON)
		tst_resm(TINFO,
			 "There are NO timing statistics produced by this test.\n\
This is because the test forks to create a child process which then calls execve.\n\
The TEST macro is NOT used.");

	TEST_PAUSE;

	tst_tmpdir();
}

void cleanup()
{
	TEST_CLEANUP;

	tst_rmdir();
}