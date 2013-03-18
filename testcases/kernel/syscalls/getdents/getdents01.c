/*
 * Copyright (c) International Business Machines  Corp., 2001
 *
 * This program is free software;  you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY;  without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
 * the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program;  if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

/*
 * ALGORITHM
 *	issue the system call
 *	check the errno value
 *	  if failure, log the errno and issue a FAIL message otherwise,
 *	  if doing functionality testing,
 *	    if the directory name of the first entry is "."
 *	      issue a PASS message
 *	    otherwise,
 *	      issue a FAIL message
 *	      break any remaining tests
 *	call cleanup
 *
 * HISTORY
 *	03/2013 - Added -l option by Markos Chandras
 *	03/2001 - Written by Wayne Boyer
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

static void cleanup(void);
static void setup(void);

char *TCID = "getdents01";
int TST_TOTAL = 1;

static int longsyscall;

static option_t options[] = {
		/* -l long option. Tests getdents64 */
		{"l", &longsyscall, NULL},
		{NULL, NULL, NULL}
};

static void help(void)
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

	if ((msg = parse_opts(ac, av, options, &help)) != NULL)
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);

	setup();

	/*
	 * Set up count to be equal to the sizeof struct dirent,
	 * just to pick a decent size.
	 */
	if (longsyscall) {
		if ((dirp64 = malloc(sizeof(struct dirent64))) == NULL)
			tst_brkm(TBROK, cleanup, "malloc failed");
		count = sizeof(struct dirent64);
	} else {
		if ((dirp = malloc(sizeof(struct dirent))) == NULL)
			tst_brkm(TBROK, cleanup, "malloc failed");
		count = sizeof(struct dirent);
	}

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		tst_count = 0;

		if ((dir_name = getcwd(dir_name, size)) == NULL)
			tst_brkm(TBROK, cleanup, "Can not get current "
				 "directory name");

		if ((fd = open(dir_name, O_RDONLY)) == -1)
			tst_brkm(TBROK, cleanup, "open of directory failed");

		if (longsyscall)
			rval = getdents64(fd, dirp64, count);
		else
			rval = getdents(fd, dirp, count);

		if (rval < 0) {
			TEST_ERROR_LOG(errno);

			tst_resm(TFAIL | TERRNO,
				 "getdents failed unexpectedly");
			continue;
		}

		if (rval == 0) {
			tst_resm(TFAIL,
				 "getdents failed - returned end of directory");
			continue;
		}

		tst_resm(TPASS, "call succeeded");

		free(dir_name);
		dir_name = NULL;

		if ((rval = close(fd)) == -1)
			tst_brkm(TBROK, cleanup, "file close failed");
	}

	cleanup();
	tst_exit();
}

static void setup(void)
{
	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	tst_tmpdir();

	TEST_PAUSE;
}

static void cleanup(void)
{
	TEST_CLEANUP;

	tst_rmdir();
}
