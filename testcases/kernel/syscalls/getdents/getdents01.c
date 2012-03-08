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
 *	getdents01.c
 *
 * DESCRIPTION
 *	getdents01 - get a directory entry
 *
 * ALGORITHM
 *	loop if that option was specified
 *	issue the system call
 *	check the errno value
 *	  if failure, log the errno and issue a FAIL message
 *	otherwise,
 *	  if doing functionality testing,
 *	    if the directory name of the first entry is "."
 *	      issue a PASS message
 *	    otherwise,
 *	      issue a FAIL message
 *	      break any remaining tests
 *	call cleanup
 *
 * USAGE:  <for command-line>
 *  getdents01 [-c n] [-f] [-i n] [-I x] [-p x] [-t]
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
 *	Any restrictions
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

char *TCID = "getdents01";
int TST_TOTAL = 1;

int main(int ac, char **av)
{
	int lc;
	char *msg;
	int rval, fd;
	int count;
	size_t size = 0;
	char *dir_name = NULL;
	struct dirent *dirp;

	if ((msg = parse_opts(ac, av, NULL, NULL)) != NULL)
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		Tst_count = 0;

		if ((dir_name = getcwd(dir_name, size)) == NULL)
			tst_brkm(TBROK, cleanup, "Can not get current "
				 "directory name");

		if ((dirp = malloc(sizeof(struct dirent))) == NULL)
			tst_brkm(TBROK, cleanup, "malloc failed");

		/*
		 * Set up count to be equal to the sizeof struct dirent,
		 * just to pick a decent size.
		 */

		count = (int)sizeof(struct dirent);

		if ((fd = open(dir_name, O_RDONLY)) == -1)
			tst_brkm(TBROK, cleanup, "open of directory failed");

		rval = getdents(fd, dirp, count);
		if (rval < 0) {

			TEST_ERROR_LOG(errno);

			tst_resm(TFAIL|TERRNO, "getdents failed unexpectedly");
			continue;
		}

		if (rval == 0) {
			tst_resm(TFAIL, "getdents failed - returned end of directory");
			continue;
		}

		tst_resm(TPASS, "call succeeded");

		free(dir_name);
		dir_name = NULL;

		free(dirp);

		if ((rval = close(fd)) == -1)
			tst_brkm(TBROK, cleanup, "file close failed");
	}

	cleanup();

	tst_exit();
}

void setup(void)
{

	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	tst_tmpdir();

	TEST_PAUSE;
}

void cleanup(void)
{

	TEST_CLEANUP;

	tst_rmdir();
}
