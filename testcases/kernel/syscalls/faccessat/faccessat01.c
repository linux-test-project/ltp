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
 *      faccessat01.c
 *
 * DESCRIPTION
 *	This test case will verify basic function of faccessat
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
#include <error.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include "test.h"
#include "linux_syscall_numbers.h"

#define TEST_CASES 6
#ifndef AT_FDCWD
#define AT_FDCWD -100
#endif
void setup();
void cleanup();
void setup_every_copy();

char *TCID = "faccessat01";
int TST_TOTAL = TEST_CASES;
char pathname[256] = "";
char testfile[256] = "";
char testfile2[256] = "";
char testfile3[256] = "";
int dirfd, fd, ret;
int fds[TEST_CASES];
char *filenames[TEST_CASES];
int expected_errno[TEST_CASES] = { 0, 0, ENOTDIR, EBADF, 0, 0 };

int myfaccessat(int dirfd, const char *filename, int mode)
{
	return ltp_syscall(__NR_faccessat, dirfd, filename, mode);
}

int main(int ac, char **av)
{
	int lc;
	int i;

	/* Disable test if the version of the kernel is less than 2.6.16 */
	if (((tst_kvercmp(2, 6, 16)) < 0)) {
		tst_resm(TWARN, "This test can only run on kernels that are ");
		tst_resm(TWARN, "2.6.16 and higher");
		exit(0);
	}

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		setup_every_copy();

		tst_count = 0;

		/*
		 * Call faccessat
		 */
		for (i = 0; i < TST_TOTAL; i++) {
			TEST(myfaccessat(fds[i], filenames[i], R_OK));

			/* check return code */
			if (TEST_ERRNO == expected_errno[i]) {
				tst_resm(TPASS,
					 "faccessat() returned the expected  errno %d: %s",
					 TEST_ERRNO,
					 strerror(TEST_ERRNO));
			} else {
				tst_resm(TFAIL,
					 "faccessdat() Failed, errno=%d : %s",
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
	sprintf(pathname, "faccessattestdir%d", getpid());
	sprintf(testfile, "faccessattestfile%d.txt", getpid());
	sprintf(testfile2, "/tmp/faccessattestfile%d.txt", getpid());
	sprintf(testfile3, "faccessattestdir%d/faccessattestfile%d.txt",
		getpid(), getpid());

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

	fds[0] = fds[1] = fds[4] = dirfd;
	fds[2] = fd;
	fds[3] = 100;
	fds[5] = AT_FDCWD;

	filenames[0] = filenames[2] = filenames[3] = filenames[4] = testfile;
	filenames[1] = testfile2;
	filenames[5] = testfile3;
}

void setup(void)
{

	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;
}

void cleanup(void)
{
	close(fd);
	unlink(testfile);
	unlink(testfile2);
	unlink(testfile3);
	rmdir(pathname);
}
