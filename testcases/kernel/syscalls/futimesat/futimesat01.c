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
 *      futimesat01.c
 *
 * DESCRIPTION
 *	This test case will verify basic function of futimesat
 *	added by kernel 2.6.16 or up.
 *
 * Author
 *	Yi Yang <yyangcdl@cn.ibm.com>
 *
 * History
 *      08/24/2006      Created first by Yi Yang <yyangcdl@cn.ibm.com>
 *
 *****************************************************************************/

#define _GNU_SOURCE

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <fcntl.h>
#include <error.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include "test.h"
#include "linux_syscall_numbers.h"

#define TEST_CASES 5
#ifndef AT_FDCWD
#define AT_FDCWD -100
#endif
void setup();
void cleanup();
void setup_every_copy();

char *TCID = "futimesat01";
int TST_TOTAL = TEST_CASES;
char pathname[256];
char testfile[256];
char testfile2[256];
char testfile3[256];
int dirfd, fd, ret;
int fds[TEST_CASES];
char *filenames[TEST_CASES];
int expected_errno[TEST_CASES] = { 0, 0, ENOTDIR, EBADF, 0 };

struct timeval times[2];

int myfutimesat(int dirfd, const char *filename, struct timeval *times)
{
	return ltp_syscall(__NR_futimesat, dirfd, filename, times);
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
		setup_every_copy();

		tst_count = 0;

		for (i = 0; i < TST_TOTAL; i++) {
			gettimeofday(&times[0], NULL);
			gettimeofday(&times[1], NULL);
			TEST(myfutimesat(fds[i], filenames[i], times));

			if (TEST_ERRNO == expected_errno[i]) {
				tst_resm(TPASS,
					 "futimesat() returned the expected  errno %d: %s",
					 TEST_ERRNO,
					 strerror(TEST_ERRNO));
			} else {
				tst_resm(TFAIL,
					 "futimesat() Failed, errno=%d : %s",
					 TEST_ERRNO, strerror(TEST_ERRNO));
			}
		}

	}

	cleanup();
	tst_exit();
}

void setup_every_copy(void)
{
	/* Initialize test dir and file names */
	sprintf(pathname, "futimesattestdir%d", getpid());
	sprintf(testfile, "futimesattestfile%d.txt", getpid());
	sprintf(testfile2, "futimesattestdir%d/futimesattestfile%d.txt",
		getpid(), getpid());
	sprintf(testfile3, "/tmp/futimesattestfile%d.txt", getpid());

	ret = mkdir(pathname, 0700);
	if (ret < 0) {
		perror("mkdir: ");
		exit(-1);
	}

	dirfd = open(pathname, O_DIRECTORY);
	if (dirfd < 0) {
		perror("open: ");
		exit(-1);
	}

	fd = open(testfile, O_CREAT | O_RDWR, 0600);
	if (fd < 0) {
		perror("open: ");
		exit(-1);
	}

	fd = open(testfile2, O_CREAT | O_RDWR, 0600);
	if (fd < 0) {
		perror("open: ");
		exit(-1);
	}

	fd = open(testfile3, O_CREAT | O_RDWR, 0600);
	if (fd < 0) {
		perror("open: ");
		exit(-1);
	}

	fds[0] = fds[1] = dirfd;
	fds[2] = fd;
	fds[3] = 100;
	fds[4] = AT_FDCWD;

	filenames[0] = filenames[2] = filenames[3] = filenames[4] = testfile;
	filenames[1] = testfile3;
}

void setup(void)
{
	tst_sig(NOFORK, DEF_HANDLER, cleanup);
	TEST_PAUSE;
}

void cleanup(void)
{
	unlink(testfile2);
	unlink(testfile3);
	unlink(testfile);
	rmdir(pathname);
}
