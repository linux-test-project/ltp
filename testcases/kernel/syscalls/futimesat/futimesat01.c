/*
 * Copyright (c) 2016 Oracle and/or its affiliates. All Rights Reserved.
 * Copyright (c) International Business Machines  Corp., 2006
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it would be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 * DESCRIPTION
 *	This test case will verify basic function of futimesat
 *	added by kernel 2.6.16 or up.
 *
 * Author
 *	Yi Yang <yyangcdl@cn.ibm.com>
 */

#define _GNU_SOURCE
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <fcntl.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include "test.h"
#include "safe_macros.h"
#include "lapi/syscalls.h"

#define TEST_CASES 5
#ifndef AT_FDCWD
#define AT_FDCWD -100
#endif

void setup();
void cleanup();

char *TCID = "futimesat01";
int TST_TOTAL = TEST_CASES;

static const char pathname[] = "futimesattestdir",
		  testfile[] = "futimesattestfile.txt",
		  testfile2[] = "futimesattestdir/futimesattestfile.txt";
static char *testfile3;

static int fds[TEST_CASES];
static const char *filenames[TEST_CASES];
static const int expected_errno[] = { 0, 0, ENOTDIR, EBADF, 0 };

int myfutimesat(int dirfd, const char *filename, struct timeval *times)
{
	return tst_syscall(__NR_futimesat, dirfd, filename, times);
}

int main(int ac, char **av)
{
	int lc, i;
	struct timeval times[2];

	if (tst_kvercmp(2, 6, 16) < 0)
		tst_brkm(TCONF, NULL, "Test must be run with kernel 2.6.16+");

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		tst_count = 0;

		for (i = 0; i < TST_TOTAL; i++) {
			gettimeofday(&times[0], NULL);
			gettimeofday(&times[1], NULL);
			TEST(myfutimesat(fds[i], filenames[i], times));

			if (TEST_ERRNO == expected_errno[i]) {
				tst_resm(TPASS | TTERRNO,
					 "futimesat() returned expected errno");
			} else {
				tst_resm(TFAIL | TTERRNO, "futimesat() failed");
			}
		}

	}

	cleanup();
	tst_exit();
}

void setup(void)
{
	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	tst_tmpdir();

	char *abs_path = tst_get_tmpdir();

	SAFE_ASPRINTF(cleanup, &testfile3, "%s/futimesatfile3.txt", abs_path);
	free(abs_path);

	SAFE_MKDIR(cleanup, pathname, 0700);

	fds[0] = SAFE_OPEN(cleanup, pathname, O_DIRECTORY);
	fds[1] = fds[0];

	SAFE_FILE_PRINTF(cleanup, testfile, testfile);
	SAFE_FILE_PRINTF(cleanup, testfile2, testfile2);

	fds[2] = SAFE_OPEN(cleanup, testfile3, O_CREAT | O_RDWR, 0600);

	fds[3] = 100;
	fds[4] = AT_FDCWD;

	filenames[0] = filenames[2] = filenames[3] = filenames[4] = testfile;
	filenames[1] = testfile3;

	TEST_PAUSE;
}

void cleanup(void)
{
	if (fds[0] > 0)
		close(fds[0]);
	if (fds[2] > 0)
		close(fds[2]);

	free(testfile3);
	tst_rmdir();
}
