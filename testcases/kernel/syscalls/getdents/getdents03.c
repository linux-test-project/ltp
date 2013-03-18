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
 *	getdents03.c
 *
 * DESCRIPTION
 *	getdents03 - check for an EINVAL error
 *
 * CALLS
 *	getdents()
 *
 * ALGORITHM
 *	loop if that option was specified
 *	issue the system call using a memory size (count) that is too small
 *	check the errno value
 *	  issue a PASS message if we get EINVAL
 *	otherwise, the tests fails
 *	  issue a FAIL message
 *	  break any remaining tests
 *	  call cleanup
 *
 * USAGE:  <for command-line>
 *  getdents03 [-c n] [-e] [-i n] [-I x] [-P x] [-t]
 *     where,  -c n : Run n copies concurrently.
 *             -e   : Turn on errno logging.
 *	       -i n : Execute test n times.
 *	       -I x : Execute test for x seconds.
 *	       -l   : Test the getdents64 system call.
 *	       -P x : Pause for x seconds between iterations.
 *	       -t   : Turn on syscall timing.
 *
 * HISTORY
 *	03/2013 - Added -l option by Markos Chandras
 *	03/2001 - Written by Wayne Boyer
 *
 * RESTRICTIONS
 *	none
 */

#include "getdents.h"
#include "test.h"
#include "usctest.h"

#include <errno.h>
#include <fcntl.h>
#include <linux/types.h>
#include <dirent.h>
#include <linux/unistd.h>
#include <unistd.h>

void cleanup(void);
void setup(void);

char *TCID = "getdents03";
int TST_TOTAL = 1;

int exp_enos[] = { EINVAL, 0 };	/* 0 terminated list of expected errnos */

static int longsyscall;

option_t Options[] = {
		{"l", &longsyscall, NULL}, /* -l long option. Tests getdents64 */
		{NULL, NULL, NULL}
};

void help(void)
{
	printf("  -l      Test the getdents64 system call\n");
}

int main(int ac, char **av)
{
	int lc;
	char *msg;
	int rval, fd;
	int count;
	size_t size = 0;
	char *dir_name = NULL;
	struct dirent64 *dirp64 = NULL;
	struct dirent *dirp = NULL;

	if ((msg = parse_opts(ac, av, Options, &help)) != NULL)
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);

	setup();

	if (longsyscall) {
		if ((dirp64 = malloc(sizeof(struct dirent64))) == NULL)
			tst_brkm(TBROK, cleanup, "malloc failed");
	} else {
		if ((dirp = malloc(sizeof(struct dirent))) == NULL)
			tst_brkm(TBROK, cleanup, "malloc failed");
	}

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		tst_count = 0;

		if ((dir_name = getcwd(dir_name, size)) == NULL)
			tst_brkm(TBROK, cleanup, "Can not get current "
				 "directory name");

		/* Set count to be very small.  The result should be EINVAL */

		count = 1;

		if ((fd = open(dir_name, O_RDONLY)) == -1)
			tst_brkm(TBROK, cleanup, "open of directory failed");

		if (longsyscall)
			rval = getdents64(fd, dirp64, count);
		else
			rval = getdents(fd, dirp, count);

		/*
		 * Hopefully we get an error due to the small buffer.
		 */

		if (rval < 0) {
			TEST_ERROR_LOG(errno);

			switch (errno) {
			case EINVAL:
				tst_resm(TPASS,
					 "getdents failed with EINVAL as expected");
				break;
			default:
				tst_resm(TFAIL | TERRNO,
					 "getdents call failed unexpectedly");
				break;
			}
		} else
			tst_resm(TFAIL, "getdents passed unexpectedly");

		free(dir_name);
		dir_name = NULL;

		if ((rval = close(fd)) == -1)
			tst_brkm(TBROK, cleanup, "fd close failed");
	}

	if (longsyscall)
		free(dirp64);
	else
		free(dirp);

	cleanup();

	tst_exit();
}

void setup(void)
{

	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;

	tst_tmpdir();

	TEST_EXP_ENOS(exp_enos);

	TEST_PAUSE;
}

void cleanup(void)
{

	TEST_CLEANUP;

	tst_rmdir();
}
