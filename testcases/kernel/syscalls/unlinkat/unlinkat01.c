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
 *	This test case will verify basic function of unlinkat
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

#define TEST_CASES 7
#ifndef AT_FDCWD
#define AT_FDCWD -100
#endif
#ifndef AT_REMOVEDIR
#define AT_REMOVEDIR 0x200
#endif

void setup();
void cleanup();

char *TCID = "unlinkat01";
int TST_TOTAL = TEST_CASES;

static const char pathname[] = "unlinkattestdir",
		  subpathname[] = "unlinkatsubtestdir",
		  subpathdir[] = "unlinkattestdir/unlinkatsubtestdir",
		  testfile[] = "unlinkattestfile.txt",
		  testfile2[] = "unlinkattestdir/unlinkattestfile.txt";
static char *testfile3;

static int fds[TEST_CASES];
static const char *filenames[TEST_CASES];
static const int expected_errno[] = { 0, 0, ENOTDIR, EBADF, EINVAL, 0, 0 };
static const int flags[] = { 0, 0, 0, 0, 9999, 0, AT_REMOVEDIR };

int myunlinkat(int dirfd, const char *filename, int flags)
{
	return ltp_syscall(__NR_unlinkat, dirfd, filename, flags);
}

int main(int ac, char **av)
{
	int lc;
	int i;

	if ((tst_kvercmp(2, 6, 16)) < 0)
		tst_brkm(TCONF, NULL, "Test must be run with kernel 2.6.16+");

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		tst_count = 0;

		for (i = 0; i < TST_TOTAL; i++) {
			TEST(myunlinkat(fds[i], filenames[i], flags[i]));

			if (TEST_ERRNO == expected_errno[i]) {
				tst_resm(TPASS | TTERRNO,
					 "unlinkat() returned expected errno");
			} else {
				tst_resm(TFAIL | TTERRNO, "unlinkat() failed");
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

	SAFE_ASPRINTF(cleanup, &testfile3, "%s/unlinkatfile3.txt", abs_path);

	free(abs_path);

	SAFE_MKDIR(cleanup, pathname, 0700);
	SAFE_MKDIR(cleanup, subpathdir, 0700);

	fds[0] = SAFE_OPEN(cleanup, pathname, O_DIRECTORY);
	fds[1] = fds[4] = fds[6] = fds[0];

	SAFE_FILE_PRINTF(cleanup, testfile, testfile);
	SAFE_FILE_PRINTF(cleanup, testfile2, testfile2);

	fds[2] = SAFE_OPEN(cleanup, testfile3, O_CREAT | O_RDWR, 0600);

	fds[3] = 100;
	fds[5] = AT_FDCWD;

	filenames[0] = filenames[2] = filenames[3] = filenames[4] =
	    filenames[5] = testfile;
	filenames[1] = testfile3;
	filenames[6] = subpathname;

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
