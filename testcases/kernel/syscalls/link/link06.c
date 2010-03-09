/*
 *
 * Copyright (C) Bull S.A. 2001
 * Copyright (c) International Business Machines  Corp., 2001
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
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

/*
 * Test Name: link06
 *
 * Test Description:
 *  Verify that, link() fails with -1 and sets errno to EACCES when
 *	 	 Write access to the directory containing newpath is
 *		 not allowed for the process's effective uid.
 *
 * Expected Result:
 *  link() should fail with return value -1 and sets expected errno.
 *
 * Algorithm:
 *  Setup:
 *   Setup signal handling.
 *   Pause for SIGUSR1 if option specified.
 *   Create temporary directory.
 *   Modify mode permissions on temporary directory: deny write access.
 *   Create regular file.
 *   Set UID to NOBODY
 *
 *  Test:
 *   Loop if the proper options are given.
 *   Execute system call
 *   Check return code, if system call failed (return=-1)
 *   	if errno set == expected errno
 *   		Issue sys call fails with expected return value and errno.
 *   	Otherwise,
 *		Issue sys call fails with unexpected errno.
 *   Otherwise,
 *	Issue sys call returns unexpected value.
 *
 *  Cleanup:
 *   Set UID to ROOT
 *   Print errno log and/or timing stats if options given
 *   Delete the temporary directory created.
 *
 * Usage:  <for command-line>
 *  link06 [-c n] [-e] [-f] [-i n] [-I x] [-P x] [-t]
 *     where,  -c n : Run n copies concurrently.
 *             -e   : Turn on errno logging.
 *             -f   : Turn off functionality Testing.
 *	       -i n : Execute test n times.
 *	       -I x : Execute test for x seconds.
 *	       -P x : Pause for x seconds between iterations.
 *	       -t   : Turn on syscall timing.
 *
 * HISTORY
 *	06/2002 Ported by Jacky Malcles
 *
 * RESTRICTIONS:
 *  none.
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pwd.h>

#include "test.h"
#include "usctest.h"

#define ROOT_USER	0
#define NOBODY_USER	99
#define MODE_TO S_IRUSR|S_IWUSR|S_IXUSR|S_IRGRP|S_IXGRP|S_IXOTH|S_IROTH

void setup();
void cleanup();

char *TCID = "link06";		/* Test program identifier.    */
int TST_TOTAL = 1;		/* Total number of test cases. */
extern int Tst_count;		/* Test Case counter for tst_* routines */

int exp_enos[] = { EACCES, 0 };
char *file1, *file2;		/* oldpath and newpath */

/***********************************************************************
 * Main
 ***********************************************************************/
int main(int ac, char **av)
{
	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */
	char *test_desc;	/* test specific error message */

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
		test_desc = "EACCES";

		/* reset Tst_count in case we are looping. */
		Tst_count = 0;

		/*
		 *  Call link(2)
		 */
		TEST(link(file1, file2));

		/* Check return code from link(2) */
		if (TEST_RETURN != -1) {
			tst_resm(TFAIL, "link() returned %ld,"
				 "expected -1, errno=%d", TEST_RETURN,
				 exp_enos[0]);
		} else {
			TEST_ERROR_LOG(TEST_ERRNO);

			if (TEST_ERRNO == exp_enos[0]) {
				tst_resm(TPASS, "link() fails with expected "
					 "error EACCES errno:%d", TEST_ERRNO);
			} else {
				tst_resm(TFAIL, "link() fails, %s, "
					 "errno=%d, expected errno=%d",
					 test_desc, TEST_ERRNO, exp_enos[0]);
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
	char Path_name[PATH_MAX];	/* Buffer to hold current path */
	int fd;
	struct passwd *nobody_pwd;

	/* capture signals */
	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	/* Pause if that option was specified */
	TEST_PAUSE;

	/* make a temp directory and cd to it */
	tst_tmpdir();
	/* Get the current working directory of the process */
	if (getcwd(Path_name, sizeof(Path_name)) == NULL) {
		tst_brkm(TBROK, cleanup,
			 "getcwd(3) fails to get working directory of process");
	}

	/* Modify mode permissions on test directory */
	if (chmod(Path_name, MODE_TO) < 0) {
		tst_brkm(TBROK, cleanup, "chmod(2) of %s failed", Path_name);
	}

	/* create regular file */
	if ((fd = creat("regfile", 0777)) == -1) {
		tst_brkm(TBROK, cleanup,
			 "creat(regfile, 0777) failed, errno:%d %s", errno,
			 strerror(errno));
	}
	close(fd);

	/* set paths for test */
	file1 = "regfile";
	file2 = "new_test_file";

	if ((nobody_pwd = getpwnam("nobody")) == NULL) {
		tst_brkm(TCONF|TERRNO, cleanup,
			"couldn't determine login information for nobody");
	}

	/* set effective user ID to "nobody"'s UID using seteuid */
	if (seteuid(nobody_pwd->pw_uid) != 0) {
		tst_brkm(TCONF|TERRNO, cleanup,
			"seteuid to %d for %s failed",
			nobody_pwd->pw_uid, nobody_pwd->pw_name);
	}

}				/* End setup() */

/***************************************************************
 * cleanup() - performs all ONE TIME cleanup for this test at
 *		completion or premature exit.
 ***************************************************************/
void cleanup()
{
	/* set back effective user ID to ROOT_USER using seteuid */
	if (seteuid(ROOT_USER) != 0) {
		tst_resm(TFAIL|TERRNO, "seteuid to %d failed", ROOT_USER);
	}

	/*
	 * print timing stats if that option was specified.
	 * print errno log if that option was specified.
	 */
	TEST_CLEANUP;

	/* Remove tmp dir and all files in it */
	tst_rmdir();

	/* exit with return code appropriate for results */
	tst_exit();
}				/* End cleanup() */
