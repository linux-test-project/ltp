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

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
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

char filename[255];
int *fd = NULL;
int nfds = 0;

int main(int ac, char **av)
{
	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */

	if ((msg = parse_opts(ac, av, NULL, NULL)) != NULL)
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {

		Tst_count = 0;

		TEST(dup(fd[0]));

		if (TEST_RETURN == -1) {
			if (STD_FUNCTIONAL_TEST) {
				if (TEST_ERRNO == EMFILE)
					tst_resm(TPASS,
					    "dup failed as expected with "
					    "EMFILE");
				else
					tst_resm(TFAIL|TTERRNO,
					    "dup failed unexpectedly");
			}
		} else {
			tst_resm(TFAIL, "dup succeeded unexpectedly");

			if (close(TEST_RETURN) == -1)
				tst_brkm(TBROK|TERRNO, cleanup,
				    "close failed");
		}

	}

	cleanup();

	tst_exit();
}

void setup()
{
	long maxfds;

	maxfds = sysconf(_SC_OPEN_MAX);
	/*
	 * Read the errors section if you're so inclined to determine
	 * why == -1 matters for errno.
	 */
	if (maxfds < 1)
		tst_brkm((maxfds == -1 ? TBROK|TERRNO : TBROK), NULL,
		    "sysconf(_SC_OPEN_MAX) failed");

	fd = malloc(maxfds * sizeof(int));
	if (fd == NULL)
		tst_brkm(TBROK|TERRNO, NULL, "malloc failed");
	fd[0] = -1;

	tst_sig(FORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;

	tst_tmpdir();

	sprintf(filename, "dupfile");
	for (nfds = 1; nfds <= maxfds; nfds++)
		if ((fd[nfds-1] = open(filename, O_RDWR|O_CREAT, 0700)) == -1) {
			if (errno == EMFILE)
				break;
			else
				tst_brkm(TBROK|TBROK, cleanup, "open failed");
			nfds--;
		}

	if (nfds == 0)
		tst_brkm(TBROK, cleanup, "unable to open at least one file");
	if (nfds > maxfds)
		tst_brkm(TBROK, cleanup,
		    "unable to open enough files to use all file descriptors, "
		    "tried %ld",
		    maxfds);
}

void cleanup()
{
	int i;

	TEST_CLEANUP;

	for (i = 3; i <= nfds; i++)
		close(fd[i]);

	tst_rmdir();
}