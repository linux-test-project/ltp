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
 * DESCRIPTION
 *	Testcase to check that open(2) sets EMFILE if a process opens files
 *	more than its descriptor size
 *
 * ALGORITHM
 *	First get the file descriptor table size which is set for a process.
 *	Use open(2) for creating files till the descriptor table becomes full.
 *	These open(2)s should succeed. Finally use open(2) to open another
 *	file. This attempt should fail with EMFILE.
 */

#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include "test.h"

char *TCID = "open04";
int TST_TOTAL = 1;

static int fd, ifile, mypid, first;
static int nfile;
static int *buf;
static char fname[40];

static void setup(void);
static void cleanup(void);

int main(int ac, char **av)
{
	int lc;

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		tst_count = 0;

		TEST(open(fname, O_RDWR | O_CREAT, 0777));

		if (TEST_RETURN != -1) {
			tst_resm(TFAIL, "call succeeded unexpectedly");
			continue;
		}

		if (TEST_ERRNO != EMFILE)
			tst_resm(TFAIL, "Expected EMFILE, got %d", TEST_ERRNO);
		else
			tst_resm(TPASS, "call returned expected EMFILE error");
	}

	close(first);
	close(fd);
	cleanup();
	tst_exit();
}

static void setup(void)
{
	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;

	/* make a temporary directory and cd to it */
	tst_tmpdir();

	mypid = getpid();
	nfile = getdtablesize();
	sprintf(fname, "open04.%d", mypid);

	first = fd = open(fname, O_RDWR | O_CREAT, 0777);
	if (first == -1)
		tst_brkm(TBROK, cleanup, "Cannot open first file");

	close(fd);
	close(first);
	unlink(fname);

	/* Allocate memory for stat and ustat structure variables */
	buf = malloc(sizeof(int) * nfile - first);
	if (buf == NULL)
		tst_brkm(TBROK, NULL, "Failed to allocate Memory");

	for (ifile = first; ifile <= nfile; ifile++) {
		sprintf(fname, "open04.%d.%d", ifile, mypid);
		fd = open(fname, O_RDWR | O_CREAT, 0777);
		if (fd == -1) {
			if (errno != EMFILE) {
				tst_brkm(TBROK, cleanup, "Expected EMFILE got "
					 "%d", errno);
			}
			break;
		}
		buf[ifile - first] = fd;
	}
}

static void cleanup(void)
{
	close(first);

	for (ifile = first; ifile < nfile; ifile++) {
		sprintf(fname, "open04.%d.%d", ifile, mypid);
		close(buf[ifile - first]);
		unlink(fname);
	}

	free(buf);

	/* delete the test directory created in setup() */
	tst_rmdir();
}
