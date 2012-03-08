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
 *	getdents04.c
 *
 * DESCRIPTION
 *	getdents04 - check for an ENOTDIR error
 *
 * CALLS
 *	getdents()
 *
 * ALGORITHM
 *	loop if that option was specified
 *	issue the system call using a file descriptor for a file
 *	check the errno value
 *	  issue a PASS message if we get ENOTDIR
 *	otherwise, the tests fails
 *	  issue a FAIL message
 *	  break any remaining tests
 *	  call cleanup
 *
 * USAGE:  <for command-line>
 *  getdents04 [-c n] [-e] [-i n] [-I x] [-P x] [-t]
 *     where,  -c n : Run n copies concurrently.
 *             -e   : Turn on errno logging.
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

void cleanup(void);
void setup(void);

char *TCID = "getdents04";
int TST_TOTAL = 1;

int exp_enos[] = { ENOTDIR, 0 };	/* 0 terminated list of expected errnos */

int main(int ac, char **av)
{
	int lc;
	char *msg;
	int count, rval, fd;
	size_t size = 0;
	char *dir_name = NULL;
	struct dirent *dirp;
	struct stat *sbuf;
	char *newfile;

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

		count = (int)sizeof(struct dirent);

		/* set up some space for a file name */
		if ((newfile = malloc(sizeof(char) * 20)) == NULL)
			tst_brkm(TBROK, cleanup, "newfile malloc failed");

		if ((rval = sprintf(newfile, "getdents03.%d", getpid())) < 0)
			tst_brkm(TBROK, cleanup, "sprintf failed");

		if ((fd = open(newfile, O_CREAT | O_RDWR, 0777)) == -1)
			tst_brkm(TBROK|TERRNO, cleanup, "open of file failed");

		/* set up some space for the stat buffer */
		if ((sbuf = malloc(sizeof(struct stat))) == NULL)
			tst_brkm(TBROK, cleanup, "stat malloc failed");

		/* make sure fd is not a directory */
		if ((rval = fstat(fd, sbuf)) == -1)
			tst_brkm(TBROK, cleanup, "fstat failed");

		if (S_ISDIR(sbuf->st_mode))
			tst_brkm(TBROK, cleanup, "fd is a directory");

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
				tst_resm(TFAIL|TERRNO,
				    "getdents failed unexpectedly");
				break;
			}
		} else
			tst_resm(TFAIL, "getdents call succeeded unexpectedly");

		free(dir_name);
		dir_name = NULL;

		free(dirp);

		if ((rval = close(fd)) == -1)
			tst_brkm(TBROK, cleanup, "fd close failed");
		if ((rval = unlink(newfile)) == -1)
			tst_brkm(TBROK, cleanup, "file unlink failed");
	}

	cleanup();

	tst_exit();
}

void setup(void)
{

	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	tst_tmpdir();

	TEST_EXP_ENOS(exp_enos);

	TEST_PAUSE;
}

void cleanup(void)
{

	TEST_CLEANUP;

	tst_rmdir();
}
