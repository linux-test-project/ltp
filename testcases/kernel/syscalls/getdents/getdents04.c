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
 *	issue the system call using a file descriptor for a file
 *	check the errno value
 *	  issue a PASS message if we get ENOTDIR
 *	otherwise, the tests fails
 *	  issue a FAIL message
 *	  break any remaining tests
 *	  call cleanup
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
#include <sys/stat.h>

static void cleanup(void);
static void setup(void);

char *TCID = "getdents04";
int TST_TOTAL = 1;

static int exp_enos[] = { ENOTDIR, 0 };

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
	int count, rval, fd;
	size_t size = 0;
	char *dir_name = NULL;
	struct stat *sbuf;
	char *newfile;
	struct dirent64 *dirp64 = NULL;
	struct dirent *dirp = NULL;

	if ((msg = parse_opts(ac, av, options, &help)) != NULL)
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);

	setup();

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

		/* set up some space for a file name */
		if ((newfile = malloc(sizeof(char) * 20)) == NULL)
			tst_brkm(TBROK, cleanup, "newfile malloc failed");

		if ((rval = sprintf(newfile, "getdents03.%d", getpid())) < 0)
			tst_brkm(TBROK, cleanup, "sprintf failed");

		if ((fd = open(newfile, O_CREAT | O_RDWR, 0777)) == -1)
			tst_brkm(TBROK | TERRNO, cleanup,
				 "open of file failed");

		/* set up some space for the stat buffer */
		if ((sbuf = malloc(sizeof(struct stat))) == NULL)
			tst_brkm(TBROK, cleanup, "stat malloc failed");

		/* make sure fd is not a directory */
		if ((rval = fstat(fd, sbuf)) == -1)
			tst_brkm(TBROK, cleanup, "fstat failed");

		if (S_ISDIR(sbuf->st_mode))
			tst_brkm(TBROK, cleanup, "fd is a directory");

		if (longsyscall)
			rval = getdents64(fd, dirp64, count);
		else
			rval = getdents(fd, dirp, count);

		/*
		 * Calling with a non directory file descriptor should give
		 * an ENOTDIR error.
		 */

		if (rval < 0) {
			TEST_ERROR_LOG(errno);

			switch (errno) {
			case ENOTDIR:
				tst_resm(TPASS,
					 "getdents failed as expected with ENOTDIR");
				break;
			default:
				tst_resm(TFAIL | TERRNO,
					 "getdents failed unexpectedly");
				break;
			}
		} else {
			tst_resm(TFAIL, "getdents call succeeded unexpectedly");
		}

		free(dir_name);
		dir_name = NULL;

		if ((rval = close(fd)) == -1)
			tst_brkm(TBROK, cleanup, "fd close failed");
		if ((rval = unlink(newfile)) == -1)
			tst_brkm(TBROK, cleanup, "file unlink failed");
	}

	cleanup();
	tst_exit();
}

static void setup(void)
{
	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	tst_tmpdir();

	TEST_EXP_ENOS(exp_enos);

	TEST_PAUSE;
}

static void cleanup(void)
{
	TEST_CLEANUP;

	tst_rmdir();
}
