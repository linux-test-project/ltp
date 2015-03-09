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
 * with this program; if not, write the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
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
/* $Id: fcntl09.c,v 1.8 2009/10/26 14:55:46 subrata_modak Exp $ */
/**********************************************************
 *
 *    OS Test - Silicon Graphics, Inc.
 *
 *    TEST IDENTIFIER	: fcntl09
 *
 *    EXECUTED BY	: anyone
 *
 *    TEST TITLE	: Basic test for fcntl(2) using F_SETLK argument.
 *
 *    PARENT DOCUMENT	: usctpl01
 *
 *    TEST CASE TOTAL	: 2
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
 *	  Print errno log and/or timing stats if options given
 *
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

void setup();
void cleanup();

char *TCID = "fcntl09";
int TST_TOTAL = 2;

char fname[255];
int fd;
struct flock flocks;

int main(int ac, char **av)
{
	int lc;

    /***************************************************************
     * parse standard options
     ***************************************************************/
	tst_parse_opts(ac, av, NULL, NULL);

    /***************************************************************
     * perform global setup for test
     ***************************************************************/
	setup();

    /***************************************************************
     * check looping state if -c option given
     ***************************************************************/
	for (lc = 0; TEST_LOOPING(lc); lc++) {
		int type;
		for (type = 0; type < 2; type++) {

			tst_count = 0;

			flocks.l_type = type ? F_RDLCK : F_WRLCK;

			/*
			 * Call fcntl(2) with F_SETLK argument on fname
			 */
			TEST(fcntl(fd, F_SETLK, &flocks));

			/* check return code */
			if (TEST_RETURN == -1) {
				tst_resm(TFAIL,
					 "fcntl(%s, F_SETLK, &flocks) flocks.l_type = %s Failed, errno=%d : %s",
					 fname, type ? "F_RDLCK" : "F_WRLCK",
					 TEST_ERRNO, strerror(TEST_ERRNO));
			} else {

				tst_resm(TPASS,
					 "fcntl(%s, F_SETLK, &flocks) flocks.l_type = %s returned %ld",
					 fname,
					 type ? "F_RDLCK" : "F_WRLCK",
					 TEST_RETURN);
			}

			flocks.l_type = F_UNLCK;
			/*
			 * Call fcntl(2) with F_SETLK argument on fname
			 */
			TEST(fcntl(fd, F_SETLK, &flocks));

			/* check return code */
			if (TEST_RETURN == -1) {
				tst_resm(TFAIL,
					 "fcntl(%s, F_SETLK, &flocks) flocks.l_type = F_UNLCK Failed, errno=%d : %s",
					 fname, TEST_ERRNO,
					 strerror(TEST_ERRNO));
			} else {
				tst_resm(TPASS,
					 "fcntl(%s, F_SETLK, &flocks) flocks.l_type = F_UNLCK returned %ld",
					 fname, TEST_RETURN);
			}
		}

	}

	cleanup();
	tst_exit();
}

/***************************************************************
 * setup() - performs all ONE TIME setup for this test.
 ***************************************************************/
void setup(void)
{

	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;

	tst_tmpdir();

	sprintf(fname, "./file_%d", getpid());
	if ((fd = creat(fname, 0644)) == -1) {
		tst_brkm(TBROK, cleanup,
			 "creat(%s, 0644) Failed, errno=%d : %s", fname, errno,
			 strerror(errno));
	} else if (close(fd) == -1) {
		tst_brkm(TBROK, cleanup, "close(%s) Failed, errno=%d : %s",
			 fname, errno, strerror(errno));
	} else if ((fd = open(fname, O_RDWR, 0700)) == -1) {
		tst_brkm(TBROK, cleanup,
			 "open(%s, O_RDWR,0700) Failed, errno=%d : %s", fname,
			 errno, strerror(errno));
	}
	flocks.l_whence = 1;
	flocks.l_start = 0;
	flocks.l_len = 0;
	flocks.l_pid = getpid();
}

/***************************************************************
 * cleanup() - performs all ONE TIME cleanup for this test at
 *		completion or premature exit.
 ***************************************************************/
void cleanup(void)
{

	if (close(fd) == -1) {
		tst_resm(TWARN, "close(%s) Failed, errno=%d : %s", fname, errno,
			 strerror(errno));
	} else if (unlink(fname) == -1) {
		tst_resm(TWARN, "unlink(%s) Failed, errno=%d : %s", fname,
			 errno, strerror(errno));

	}

	tst_rmdir();

}
