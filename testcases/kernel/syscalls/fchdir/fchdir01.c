/*
 *
 *   Copyright (c) International Business Machines  Corp., 2001
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
 * NAME
 *	fchdir01.c
 *
 * DESCRIPTION
 *	fchdir01 - create a directory and cd into it.
 *
 * ALGORITHM
 *	create a new directory
 *	open the directory and get a file descriptor
 *	loop if that option was specified
 *	fchdir() into the directory
 *	check the return code
 *	  if failure, issue a FAIL message.
 *	otherwise,
 *	  if doing functionality testing, call check_functionality()
 *	  	if correct,
 *			issue a PASS message
 *		otherwise
 *			issue a FAIL message
 *	call cleanup
 *
 * USAGE:  <for command-line>
 *  fchdir01 [-c n] [-f] [-i n] [-I x] [-P x] [-t]
 *     where,  -c n : Run n copies concurrently.
 *             -f   : Turn off functionality Testing.
 *	       -i n : Execute test n times.
 *	       -I x : Execute test for x seconds.
 *	       -P x : Pause for x seconds between iterations.
 *	       -t   : Turn on syscall timing.
 *
 * HISTORY
 *	03/2001 - Written by Wayne Boyer
 *
 * RESTRICTIONS
 *	none
 */

#include "test.h"
#include "usctest.h"

#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

void cleanup(void);
void setup(void);

char *TCID = "fchdir01";
int TST_TOTAL = 1;
extern int Tst_count;

int fd;
char *temp_dir;
const char *TEST_DIR = "alpha";

#define MODES	S_IRWXU

int main(int ac, char **av)
{
	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */
	void check_functionality(void);
	int r_val;

	/* parse standard options */
	if ((msg = parse_opts(ac, av, (option_t *) NULL, NULL)) != (char *)NULL) {
		tst_brkm(TBROK, cleanup, "OPTION PARSING ERROR - %s", msg);
	}

	setup();		/* global setup */

	/* The following loop checks looping state if -i option given */

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		/* reset Tst_count in case we are looping */
		Tst_count = 0;

		/* get the name of the test dirctory */
		if ((temp_dir = (getcwd(temp_dir, 0))) == NULL) {
			tst_brkm(TBROK, cleanup, "%s - getcwd() in main() "
				 "failed", TCID);
		}

		/*
		 * create a new directory and open it
		 */

		if ((r_val = mkdir(TEST_DIR, MODES)) == -1) {
			tst_brkm(TBROK, cleanup, "%s - mkdir() in main() "
				 "failed", TCID);
		}

		if ((fd = open(TEST_DIR, O_RDONLY)) == -1) {
			tst_brkm(TBROK, cleanup, "open of directory failed");
		}

		/*
		 * Use TEST macro to make the call
		 */

		TEST(fchdir(fd));

		if (TEST_RETURN == -1) {
			tst_brkm(TFAIL, cleanup, "%s call failed - errno = %d :"
				 " %s", TCID, TEST_ERRNO, strerror(TEST_ERRNO));
		} else {
			if (STD_FUNCTIONAL_TEST) {
				check_functionality();
			} else {
				tst_resm(TPASS, "call succeeded");
			}
		}

		/*
		 * clean up things in case we are looping
		 */

		/*
		 * NOTE: in case of failure here, we need to use "tst_resm()"
		 * and not "tst_brkm()".  This is because if we get to this
		 * point, we have already set a PASS or FAIL for the test
		 * and "tst_brkm()" won't report as we might expect.
		 */

		/* chdir back to our temporary work directory */
		if ((r_val = chdir("..")) == -1) {
			tst_resm(TBROK, "fchdir failed - errno = %d : %s",
				 errno, strerror(errno));
		}

		if ((r_val = rmdir(TEST_DIR)) == -1) {
			tst_resm(TBROK, "rmdir failed - errno = %d : %s",
				 errno, strerror(errno));
		}

		/*
		 * clean up things in case we are looping
		 */
		free(temp_dir);
		temp_dir = NULL;
	}

	cleanup();

	 /*NOTREACHED*/ return 0;
}

/*
 * check_functionality() - check that we are in the correct directory.
 */
void check_functionality(void)
{
	char *buf = NULL;
	char **bufptr = &buf;
	char *dir;

	/*
	 * Get the current directory path.
	 */
	if ((buf = (getcwd(buf, 0))) == NULL) {
		tst_brkm(TBROK, cleanup, "%s - getcwd() in "
			 "check_functionality() failed", TCID);
	}

	/*
	 * strip off all but the last directory name in the
	 * current working directory.
	 */
	do {
		if ((dir = strsep(bufptr, "/")) == NULL) {
			tst_brkm(TBROK, cleanup, "%s - strsep() in "
				 "check_functionality() failed", TCID);
		}
	} while (*bufptr != NULL);

	/*
	 * Make sure we are in the right place.
	 */
	if (strcmp(TEST_DIR, dir) == 0) {
		tst_resm(TPASS, "%s call succeeded", TCID);
	} else {
		tst_resm(TFAIL, "%s functionality test failed", TCID);
	}
}

/*
 * setup() - performs all the ONE TIME setup for this test.
 */
void setup(void)
{
	/* capture signals */
	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	/* Pause if that option was specified */
	TEST_PAUSE;

	/* create a test directory and cd into it */
	tst_tmpdir();
}

/*
 * cleanup() - performs all the ONE TIME cleanup for this test at completion
 * 	       or premature exit.
 */
void cleanup(void)
{
	/* remove the test directory */
	tst_rmdir();

	/*
	 * print timing stats if that option was specified.
	 * print errno log if that option was specified.
	 */
	TEST_CLEANUP;

	/* exit with return code appropriate for results */
	tst_exit();
}
