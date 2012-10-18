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
 * 	chdir01.c
 *
 * DESCRIPTION
 *	Check proper operation of chdir(): tests whether the
 *	system call can it change the current, working directory, and find a
 *	file there? Will it fail on a non-directory entry ?
 *
 * ALGORITHM
 * 	Make a directory "Testdirectory", and create a file in it. cd into
 * 	the directory and read the entry. It should be the file just
 * 	created.
 *
 * USAGE:  <for command-line>
 *  chdir01 [-c n] [-e] [-i n] [-I x] [-P x] [-t]
 *     where,  -c n : Run n copies concurrently.
 *             -e   : Turn on errno logging.
 *             -i n : Execute test n times.
 *             -I x : Execute test for x seconds.
 *             -P x : Pause for x seconds between iterations.
 *             -t   : Turn on syscall timing.
 *
 * HISTORY
 *	07/2001 Ported by Wayne Boyer
 *
 * RESTRICTIONS
 * 	None
 */

#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include<sys/stat.h>
#include "test.h"
#include "usctest.h"
#include "safe_macros.h"

char *TCID = "chdir01";		/* Test program identifier */
int TST_TOTAL = 1;		/* Total number of test cases */

int exp_enos[] = { ENOTDIR, 0 };

void setup(void);
void cleanup(void);
static void checknames(char **, int, DIR *);

char testdir[40] = "";

int main(int ac, char **av)
{
	DIR *ddir, *opendir();
	int fd;
	char *filname = "chdirtest";
	char *filenames[3];

	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */

	/* parse standard options */
	if ((msg = parse_opts(ac, av, NULL, NULL)) != NULL)
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);

	setup();

	TEST_EXP_ENOS(exp_enos);

	for (lc = 0; TEST_LOOPING(lc); lc++) {

		Tst_count = 0;

		SAFE_CHDIR(cleanup, testdir);

		fd = SAFE_CREAT(cleanup, filname, 0000);
		SAFE_CLOSE(cleanup, fd);
		if ((ddir = opendir(".")) == NULL)
			tst_brkm(TBROK|TERRNO, cleanup, "opendir(.) failed");

		filenames[0] = ".";
		filenames[1] = "..";
		filenames[2] = filname;
		checknames(filenames, sizeof(filenames) / sizeof(filenames[0]),
		    ddir);
		closedir(ddir);

		TEST(chdir(filname));

		if (TEST_RETURN != -1)
			tst_resm(TFAIL, "call succeeded unexpectedly");
		else if (TEST_ERRNO != ENOTDIR)
			tst_resm(TFAIL|TTERRNO,
			    "failed unexpectedly; wanted ENOTDIR");
		else
			tst_resm(TPASS,
			    "failed as expected with ENOTDIR");

		if (unlink(filname) == -1)
			tst_brkm(TBROK|TERRNO, cleanup, "Couldn't remove file");

		SAFE_CHDIR(cleanup, "..");

	}
	cleanup();

	tst_exit();

}

void setup(void)
{

	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	umask(0);

	TEST_PAUSE;

	tst_tmpdir();

	sprintf(testdir, "Testdir.%d", getpid());

	SAFE_MKDIR(cleanup, testdir, 0700);
}

void cleanup(void)
{
	TEST_CLEANUP;

	tst_rmdir();
}

void checknames(char **pfilnames, int fnamecount, DIR *ddir)
{
	struct dirent *dir;
	int i, found;

	found = 0;
	while ((dir = readdir(ddir)) != NULL) {
		for (i = 0; i < fnamecount; i++) {
			/*
			 * if dir->d_name is not null terminated it is a bug
			 * anyway
			 */
			if (strcmp(pfilnames[i], dir->d_name) == 0) {
				tst_resm(TINFO, "Found file %s", dir->d_name);
				found++;
			}
		}
	}
	if (found < fnamecount) {
		tst_brkm(TFAIL, cleanup,
			 "Some files do not exist, but they must exist");
	}
}
