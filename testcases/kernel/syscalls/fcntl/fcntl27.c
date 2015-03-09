/*
 *
 *   Copyright (C) Bull S.A. 2005
 *   Copyright (c) International Business Machines  Corp., 2004
 *
 *   This program is free software;  you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY;  without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
 *   the GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program;  if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

/**********************************************************
 *
 *    TEST IDENTIFIER		 : fcntl27
 *
 *    EXECUTED BY		 : anyone
 *
 *    TEST TITLE		 : Basic test for fcntl(2) using F_SETLEASE & F_RDLCK argument.
 *
 *    TEST CASE TOTAL		 : 1
 *
 *    WALL CLOCK TIME		 : 1
 *
 *    CPU TYPES				 : ALL
 *
 *    AUTHOR				 : Jacky Malcles
 *
 *    TEST CASES
 *
 *		 1.) fcntl(2) returns...(See Description)
 *
 *    INPUT SPECIFICATIONS
 *		 The standard options for system call tests are accepted.
 *		 (See the parse_opts(3) man page).
 *
 *    OUTPUT SPECIFICATIONS
 *
 *    DURATION
 *		 Terminates - with frequency and infinite modes.
 *
 *    SIGNALS
 *		 Uses SIGUSR1 to pause before test if option set.
 *		 (See the parse_opts(3) man page).
 *
 *    RESOURCES
 *		 None
 *
 *    ENVIRONMENTAL NEEDS
 *      No run-time environmental needs.
 *
 *    SPECIAL PROCEDURAL REQUIREMENTS
 *		 None
 *
 *    INTERCASE DEPENDENCIES
 *		 None
 *
 *    DETAILED DESCRIPTION
 *		 This is a Phase I test for the fcntl(2) system call.  It is intended
 *		 to provide a limited exposure of the system call, for now.  It
 *		 should/will be extended when full functional tests are written for
 *		 fcntl(2).
 *
 *		 Setup:
 *		   Setup signal handling.
 *		   Pause for SIGUSR1 if option specified.
 *
 *		 Test:
 *		  Loop if the proper options are given.
 *		   Execute system call
 *		   Check return code, if system call failed (return=-1)
 *				 Log the errno and Issue a FAIL message.
 *		   Otherwise, Issue a PASS message.
 *
 *		 Cleanup:
 *		   Print errno log and/or timing stats if options given
 *
 *
 *#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#**/

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include "test.h"

void setup();
void cleanup();

char *TCID = "fcntl27";
int TST_TOTAL = 1;

char fname[255];
int fd;

int main(int ac, char **av)
{
	int lc, expected_result = -1;
    /***************************************************************
     * parse standard options
     ***************************************************************/
	tst_parse_opts(ac, av, NULL, NULL);

    /***************************************************************
     * perform global setup for test
     ***************************************************************/
	setup();

	expected_result = -1;

    /***************************************************************
     * check looping state if -c option given
     ***************************************************************/
	for (lc = 0; TEST_LOOPING(lc); lc++) {

		tst_count = 0;

#ifdef F_SETLEASE
		/*
		 * Call fcntl(2) with F_SETLEASE & F_RDLCK argument on fname
		 */
		TEST(fcntl(fd, F_SETLEASE, F_RDLCK));

		/* check return code */
		if (TEST_RETURN == expected_result) {
			tst_resm(TPASS,
				 "fcntl(fd, F_SETLEASE, F_RDLCK) succeeded");
		} else {
			tst_resm(TFAIL, "fcntl(%s, F_SETLEASE, F_RDLCK)"
				 " failed with errno %d : %s", fname,
				 TEST_ERRNO, strerror(TEST_ERRNO));
		}
#else
		tst_resm(TINFO, "F_SETLEASE not defined, skipping test");
#endif
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
void setup(void)
{

	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;

	tst_tmpdir();

	sprintf(fname, "tfile_%d", getpid());
	if ((fd = open(fname, O_RDWR | O_CREAT, 0777)) == -1) {
		tst_brkm(TBROK, cleanup,
			 "open(%s, O_RDWR|O_CREAT,0777) Failed, errno=%d : %s",
			 fname, errno, strerror(errno));
	}
}

/***************************************************************
 * cleanup() - performs all ONE TIME cleanup for this test at
 *				 completion or premature exit.
 ***************************************************************/
void cleanup(void)
{

	/* close the file we've had open */
	if (close(fd) == -1) {
		tst_resm(TWARN, "close(%s) Failed, errno=%d : %s", fname, errno,
			 strerror(errno));
	}

	tst_rmdir();

}
