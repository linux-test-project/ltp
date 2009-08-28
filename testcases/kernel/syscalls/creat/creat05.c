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
 *	creat05.c
 *
 * DESCRIPTION
 *	Testcase to check that creat(2) system call returns EMFILE.
 *
 * ALGORITHM
 *	A process creates a temporary test directory, then it should
 *	get the system configured NOFILE parameter (ie. the maximum no. of
 *	open files of a process). This can be done using the getdtablesize()
 *	system call. Once this is determined, the process should attempt
 *	to creat(2) the this no. of files (can be done using a "for" loop).
 *	Then, the process should attempt to creat() another file, which should
 *	fail with EMFILE.
 *
 * USAGE
 *	creat05
 *
 * HISTORY
 *	07/2001 Ported by Wayne Boyer
 *
 * RESTRICTIONS
 *	None
 */

#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <linux/limits.h>
#include <unistd.h>
#include <test.h>
#include <usctest.h>

char *TCID = "creat05";
int TST_TOTAL = 1;
extern int Tst_count;

#define MODE	0666

void remove_files(int);
void setup(void);
void cleanup(void);

int exp_enos[] = { EMFILE, 0 };

int fd, ifile, mypid, first;
int *buf;
char fname[40];

int main(int ac, char **av)
{
	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */

	/* parse standard options */
	if ((msg = parse_opts(ac, av, (option_t *) NULL, NULL)) != (char *)NULL) {
		tst_brkm(TBROK, cleanup, "OPTION PARSING ERROR - %s", msg);
	}

	setup();

	TEST_EXP_ENOS(exp_enos);

	/* check looping state if -i option given */
	for (lc = 0; TEST_LOOPING(lc); lc++) {

		/* reset Tst_count in case we are looping */
		Tst_count = 0;

		TEST(creat(fname, MODE));

		if (TEST_RETURN != -1) {
			tst_resm(TFAIL, "call succeeded unexpectedly");
			continue;
		}

		TEST_ERROR_LOG(TEST_ERRNO);

		if (TEST_ERRNO == EMFILE) {
			tst_resm(TPASS, "call failed with expected error - "
				 "EMFILE");
		} else {
			tst_resm(TFAIL|TTERRNO, "Expected EMFILE");
		}

		remove_files(ifile);
	}
	cleanup();

	return 0;
 /*NOTREACHED*/}

/*
 * setup() - performs all ONE TIME setup for this test.
 */
void setup()
{
	int max_open;

	/* capture signals */
	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	/* Pause if that option was specified */
	TEST_PAUSE;

	/* make a temporary directory and cd to it */
	tst_tmpdir();

	umask(0);
	mypid = getpid();
	sprintf(fname, "creat05.%d", mypid);
	unlink(fname);

	/* create a file to get the first file descriptor available */
	if ((first = fd = creat(fname, MODE)) == -1) {
		tst_brkm(TFAIL, cleanup, "Cannot open first file");
	}

	close(fd);
	unlink(fname);
	tst_resm(TINFO, "first file is #%d", fd);

	/* get the maximum number of files that we can open */
	max_open = getdtablesize();
	/* Allocate memory for stat and ustat structure variables */
	if ((buf = (int *)malloc(sizeof(int) * max_open - first)) == NULL) {
		tst_brkm(TBROK, tst_exit, "Failed to allocate Memory");
	}

	/* now open as many files as we can up to max_open */
	for (ifile = first; ifile <= max_open; ifile++) {
		sprintf(fname, "creat05.%d.%d", ifile, mypid);
		if ((fd = creat(fname, 0666)) == -1) {
			tst_resm(TINFO, "could not creat file "
				 "#%d", ifile + 1);
			if (errno != EMFILE) {
				remove_files(ifile);
				tst_brkm(TBROK, cleanup, "Expected "
					 "EMFILE got %d", errno);
			}
			break;
		}
		buf[ifile - first] = fd;
	}
}

/*
 * remove_files - remove the temporary files that were created
 */
void remove_files(int nfiles)
{
	int i;

	for (i = first; i < nfiles; i++) {
		sprintf(fname, "creat05.%d.%d", i, mypid);
		close(buf[i - first]);
		unlink(fname);
	}
}

/*
 * cleanup() - performs all ONE TIME cleanup for this test at
 *	       completion or premature exit.
 */
void cleanup()
{
	/*
	 * print timing stats if that option was specified.
	 * print errno log if that option was specified.
	 */
	close(first);

	TEST_CLEANUP;

	/* delete the test directory created in setup() */
	tst_rmdir();

	/* exit with return code appropriate for results */
	tst_exit();
}
