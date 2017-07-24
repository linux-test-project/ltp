/******************************************************************************
 *
 *   Copyright (c) International Business Machines  Corp., 2006
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
 *
 * NAME
 *      fchmodat01.c
 *
 * DESCRIPTION
 *	This test case will verify basic function of fchmodat
 *	added by kernel 2.6.16 or up.
 *
 * Author
 *	Yi Yang <yyangcdl@cn.ibm.com>
 *
 * History
 *      08/28/2006      Created first by Yi Yang <yyangcdl@cn.ibm.com>
 *
 *****************************************************************************/

#define _GNU_SOURCE

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include "test.h"
#include "safe_macros.h"
#include "lapi/syscalls.h"

#define TEST_CASES 6
#ifndef AT_FDCWD
#define AT_FDCWD -100
#endif
void setup();
void cleanup();

char *TCID = "fchmodat01";
int TST_TOTAL = TEST_CASES;
char pathname[256];
char testfile[256];
char testfile2[256];
char testfile3[256];
int fds[TEST_CASES];
char *filenames[TEST_CASES];
int expected_errno[TEST_CASES] = { 0, 0, ENOTDIR, EBADF, 0, 0 };

int myfchmodat(int dirfd, const char *filename, mode_t mode)
{
	return ltp_syscall(__NR_fchmodat, dirfd, filename, mode);
}

int main(int ac, char **av)
{
	int lc;
	int i;

	/* Disable test if the version of the kernel is less than 2.6.16 */
	if ((tst_kvercmp(2, 6, 16)) < 0) {
		tst_resm(TWARN, "This test can only run on kernels that are ");
		tst_resm(TWARN, "2.6.16 and higher");
		exit(0);
	}

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		tst_count = 0;

		for (i = 0; i < TST_TOTAL; i++) {
			TEST(myfchmodat(fds[i], filenames[i], 0600));

			if (TEST_ERRNO == expected_errno[i]) {
				tst_resm(TPASS,
					 "fchmodat() returned the expected  errno %d: %s",
					 TEST_ERRNO, strerror(TEST_ERRNO));
			} else {
				tst_resm(TFAIL,
					 "fchmodat() Failed, errno=%d : %s",
					 TEST_ERRNO, strerror(TEST_ERRNO));
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

	/* Initialize test dir and file names */
	char *abs_path = tst_get_tmpdir();
	int p = getpid();

	sprintf(pathname, "fchmodattestdir%d", p);
	sprintf(testfile, "fchmodattest%d.txt", p);
	sprintf(testfile2, "%s/fchmodattest%d.txt", abs_path, p);
	sprintf(testfile3, "fchmodattestdir%d/fchmodattest%d.txt", p, p);

	free(abs_path);

	SAFE_MKDIR(cleanup, pathname, 0700);

	fds[0] = SAFE_OPEN(cleanup, pathname, O_DIRECTORY);
	fds[1] = fds[4] = fds[0];

	SAFE_FILE_PRINTF(cleanup, testfile, "%s", testfile);
	SAFE_FILE_PRINTF(cleanup, testfile2, "%s", testfile2);

	fds[2] = SAFE_OPEN(cleanup, testfile3, O_CREAT | O_RDWR, 0600);
	fds[3] = 100;
	fds[5] = AT_FDCWD;

	filenames[0] = filenames[2] = filenames[3] = filenames[4] = testfile;
	filenames[1] = testfile2;
	filenames[5] = testfile3;

	TEST_PAUSE;
}

void cleanup(void)
{
	if (fds[0] > 0)
		close(fds[0]);
	if (fds[2] > 0)
		close(fds[2]);

	tst_rmdir();
}
