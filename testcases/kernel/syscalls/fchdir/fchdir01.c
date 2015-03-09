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
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
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

#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <libgen.h>
#include <string.h>

void cleanup(void);
void setup(void);

char *TCID = "fchdir01";
int TST_TOTAL = 1;

int fd;
char *temp_dir;
const char *TEST_DIR = "alpha";

#define MODES	S_IRWXU

int main(int ac, char **av)
{
	int lc;
	void check_functionality(void);
	int r_val;

	tst_parse_opts(ac, av, NULL, NULL);

	setup();		/* global setup */

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		tst_count = 0;

		/* get the name of the test dirctory */
		if ((temp_dir = (getcwd(temp_dir, 0))) == NULL)
			tst_brkm(TBROK, cleanup, "getcwd failed");

		/*
		 * create a new directory and open it
		 */

		if ((r_val = mkdir(TEST_DIR, MODES)) == -1)
			tst_brkm(TBROK, cleanup, "mkdir failed");

		if ((fd = open(TEST_DIR, O_RDONLY)) == -1)
			tst_brkm(TBROK, cleanup, "open of directory failed");

		TEST(fchdir(fd));

		if (TEST_RETURN == -1) {
			tst_brkm(TFAIL | TTERRNO, cleanup,
				 "fchdir call failed");
		} else {
				check_functionality();
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
		if ((r_val = chdir("..")) == -1)
			tst_resm(TBROK | TERRNO, "chdir failed");

		if ((r_val = rmdir(TEST_DIR)) == -1)
			tst_resm(TBROK | TERRNO, "rmdir failed");

		free(temp_dir);
		temp_dir = NULL;
	}

	cleanup();
	tst_exit();
}

void check_functionality(void)
{
	char *buf = NULL;
	char *dir;

	if ((buf = (getcwd(buf, 0))) == NULL) {
		tst_brkm(TBROK, cleanup, "getcwd failed");
	}

	if ((dir = basename(buf)) == NULL)
		tst_brkm(TBROK, cleanup, "basename failed");

	if (strcmp(TEST_DIR, dir) == 0)
		tst_resm(TPASS, "fchdir call succeeded");
	else
		tst_resm(TFAIL, "fchdir call failed");
}

void setup(void)
{

	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;

	tst_tmpdir();
}

void cleanup(void)
{
	tst_rmdir();
}
