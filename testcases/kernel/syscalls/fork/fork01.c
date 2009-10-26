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
/* $Id: fork01.c,v 1.6 2009/10/26 14:55:46 subrata_modak Exp $ */
/**********************************************************
 *
 *    OS Test - Silicon Graphics, Inc.
 *
 *    TEST IDENTIFIER	: fork01
 *
 *    EXECUTED BY	: anyone
 *
 *    TEST TITLE	: Basic test for fork(2)
 *
 *    PARENT DOCUMENT	: frktds02
 *
 *    TEST CASE TOTAL	: 2
 *
 *    WALL CLOCK TIME	: 1
 *
 *    CPU TYPES		: ALL
 *
 *    AUTHOR		: Kathy Olmsted
 *
 *    CO-PILOT		: Steve Shaw
 *
 *    DATE STARTED	: 06/17/92
 *
 *    INITIAL RELEASE	: UNICOS 7.0
 *
 *    TEST CASES
 *
 * 	1.) fork returns without error
 *      2.) fork returns the pid of the child
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
 *        fork()
 *	  Check return code, if system call failed (return=-1)
 *		Log the errno and Issue a FAIL message.
 *	  Otherwise, Issue a PASS message.
 *        CHILD:
 *           determine PID
 *           write to PID to a file and close the file
 *           exit
 *        PARENT:
 *           wait for child to exit
 *           read child PID from file
 *           compare child PID to fork() return code and report
 *           results
 *
 * 	Cleanup:
 * 	  Print errno log and/or timing stats if options given
 *
 *
 *#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#**/

#include <errno.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "test.h"
#include "usctest.h"

#define	KIDEXIT	42
void setup();
void cleanup();

#define LINE_SZ	20		/* size of the line written/read to the file */
#define FILENAME	"childpid"

char *TCID = "fork01";		/* Test program identifier.    */
int TST_TOTAL = 2;		/* Total number of test cases. */
extern int Tst_count;		/* Test Case counter for tst_* routines */

/***************************************************************
 * child_pid - the child side of the test
 *        determine the PID and write to a file
 ***************************************************************/
void child_pid()
{

	int fildes;
	char tmp_line[LINE_SZ];

	fildes = creat(FILENAME, 0700);
	sprintf(tmp_line, "%d\n", getpid());
	write(fildes, tmp_line, LINE_SZ);
	close(fildes);

}

/***************************************************************
 * parent_pid - the parent side of the test
 *        read the value determined by the child
 *        compare and report results
 ***************************************************************/
void parent_pid()
{

	int fildes;
	char tmp_line[LINE_SZ];
	pid_t child_id;

	if ((fildes = open(FILENAME, O_RDWR)) == -1) {
		tst_brkm(TBROK, cleanup,
			 "parent open failed. errno: %d (%s)\n",
			 errno, strerror(errno));
	} else {
		if (read(fildes, tmp_line, LINE_SZ) == 0) {
			tst_brkm(TBROK, cleanup,
				 "fork(): parent failed to read PID from file errno: %d (%s)",
				 errno, strerror(errno));
		} else {
			child_id = atoi(tmp_line);
			if (TEST_RETURN != child_id) {
				tst_resm(TFAIL,
					 "child reported a pid of %d. parent received %ld from fork()",
					 child_id, TEST_RETURN);
			} else {
				tst_resm(TPASS,
					 "child pid and fork() return agree: %d",
					 child_id);
			}
		}
		close(fildes);
	}
}

/***************************************************************
 * main() - performs tests
 *
 ***************************************************************/

int main(int ac, char **av)
{
	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */
	int fails;
	int kid_status, wait_status;

    /***************************************************************
     * parse standard options
     ***************************************************************/
	if ((msg = parse_opts(ac, av, (option_t *) NULL, NULL)) != (char *)NULL)
		tst_brkm(TBROK, cleanup, "OPTION PARSING ERROR - %s", msg);

    /***************************************************************
     * perform global setup for test
     ***************************************************************/
	setup();

	/* set the expected errnos... */
    /***************************************************************
     * check looping state if -c option given
     ***************************************************************/
	for (lc = 0; TEST_LOOPING(lc); lc++) {

		/* reset Tst_count in case we are looping. */
		Tst_count = 0;
		fails = 0;

		/*
		 * Call fork(2)
		 */
		TEST(fork());

		/* check return code */
		if (TEST_RETURN == -1) {
			TEST_ERROR_LOG(TEST_ERRNO);
			if (STD_FUNCTIONAL_TEST) {
				tst_resm(TFAIL, "fork() Failed, errno=%d : %s",
					 TEST_ERRNO, strerror(TEST_ERRNO));
				tst_resm(TBROK, "unable to continue");
			}
		}
		if (TEST_RETURN == 0) {
			/* child */
			if (STD_FUNCTIONAL_TEST) {
				child_pid();
			}
			exit(KIDEXIT);
		} else {
			/* parent */
			if (STD_FUNCTIONAL_TEST) {
				tst_resm(TPASS, "fork() returned %ld",
					 TEST_RETURN);
			}
			/* wait for the child to complete */
			wait_status = waitpid(TEST_RETURN, &kid_status, 0);
			if (STD_FUNCTIONAL_TEST) {
				if (wait_status == TEST_RETURN) {
					if (kid_status != KIDEXIT << 8) {
						tst_resm(TBROK,
							 "incorrect child status returned on wait(): %d",
							 kid_status);
						fails++;
					}
				} else {
					tst_resm(TBROK,
						 "wait() for child status failed with %d errno: %d : %s",
						 wait_status, errno,
						 strerror(errno));
					fails++;
				}
				if (fails == 0) {
					/* verification tests */
					parent_pid();
				}
			}	/* STD_FUNCTIONAL_TEST */
		}		/* TEST_RETURN */
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

	tst_tmpdir();
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
	tst_rmdir();
	tst_exit();
}				/* End cleanup() */
