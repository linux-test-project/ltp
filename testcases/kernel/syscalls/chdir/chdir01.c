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

char *TCID = "chdir01";		/* Test program identifier */
int TST_TOTAL = 1;		/* Total number of test cases */
extern int Tst_count;		/* Test case counter */

int exp_enos[] = { ENOTDIR, 0 };

void setup(void);
void cleanup(void);
static void checknames(char **, int, DIR *);

char testdir[40] = "";

int main(int ac, char **av)
{
	DIR *ddir, *opendir();
	int fd, ret;
	char *filname = "chdirtest";
	char *filenames[3];

	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */

	/* parse standard options */
	if ((msg = parse_opts(ac, av, (option_t *) NULL, NULL)) != (char *)NULL) {
		tst_brkm(TBROK, cleanup, "OPTION PARSING ERROR - %s", msg);
	}

	setup();		/* global setup */

	/* set up expected errnos */
	TEST_EXP_ENOS(exp_enos);

	/* Check for looping state if -i option is given */
	for (lc = 0; TEST_LOOPING(lc); lc++) {

		/* reset Tst_count in case we are looping */
		Tst_count = 0;

		if ((ret = chdir(testdir)) != 0) {
			tst_brkm(TBROK|TERRNO, cleanup, "chdir(%s) failed", testdir);
		 /*NOTREACHED*/}
		if ((fd = creat(filname, 0000)) == -1) {
			tst_brkm(TBROK|TERRNO, cleanup, "creat(%s) failed", filname);
		 /*NOTREACHED*/}
		if ((ddir = opendir(".")) == NULL) {
			tst_brkm(TBROK|TERRNO, cleanup, "opendir(.) failed");
		 /*NOTREACHED*/}

		filenames[0] = ".";
		filenames[1] = "..";
		filenames[2] = filname;
		checknames(filenames, 3, ddir);

		TEST(chdir(filname));

		if (TEST_RETURN != -1) {
			tst_resm(TFAIL, "call succeeded on expected fail");
		} else if (TEST_ERRNO != ENOTDIR) {
			tst_resm(TFAIL|TTERRNO, "received unexpected errno (wanted ENOTDIR)");
		} else {
			TEST_ERROR_LOG(TEST_ERRNO);
			tst_resm(TPASS|TTERRNO, "received expected error");
		}

		/* reset things in case we are looping */

		/* remove created file */
		if (unlink(filname) == -1) {
			tst_brkm(TBROK, cleanup, "Couldn't remove file");
		}

		/* cd back to starting directory */
		chdir("..");

	}
	close(fd);
	cleanup();

	return 0;
 /*NOTREACHED*/}

/*
 * setup() - performs all ONE TIME setup for this test
 */
void setup(void)
{
	/* capture signals */
	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	umask(0);

	/* Pause if that option was specified */
	TEST_PAUSE;

	/* make a temporary directory and cd to it */
	tst_tmpdir();

	sprintf(testdir, "Testdir.%d", getpid());

	if (mkdir(testdir, 0700) == -1) {
		tst_brkm(TBROK|TERRNO, cleanup, "mkdir(%s) failed", testdir);
	}
}

/*
 * cleanup() - performs all the ONE TIME cleanup for this test at
 *	       completion or premature exit.
 */
void cleanup(void)
{
	/*
	 * print timing status if that option was specified
	 * print errno log if that option was specified
	 */
	TEST_CLEANUP;

	/* Delete the test directory created in setup(). */
	tst_rmdir();

	/* exit with return code appropriate for results */
	tst_exit();
}

void checknames(pfilnames, fnamecount, ddir)
char **pfilnames;
int fnamecount;
DIR *ddir;
{
	struct dirent *dir;
	int i, found;

	found = 0;
	while ((dir = readdir(ddir)) != (struct dirent *)0) {
		for (i = 0; i < fnamecount; i++) {
			/* if dir->d_name is not null terminated it is a bug anyway */
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
