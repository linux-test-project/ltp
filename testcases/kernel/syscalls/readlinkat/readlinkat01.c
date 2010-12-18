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
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 * NAME
 *      readlinkat01.c
 *
 * DESCRIPTION
 *	This test case will verify basic function of readlinkat
 *	added by kernel 2.6.16 or up.
 *
 * USAGE:  <for command-line>
 * readlinkat01 [-c n] [-e] [-i n] [-I x] [-P x] [-t] [-p]
 * where:
 *      -c n : Run n copies simultaneously.
 *      -e   : Turn on errno logging.
 *      -i n : Execute test n times.
 *      -I x : Execute test for x seconds.
 *      -p   : Pause for SIGUSR1 before starting
 *      -P x : Pause for x seconds between iterations.
 *      -t   : Turn on syscall timing.
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
#include <sys/time.h>
#include <fcntl.h>
#include <error.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include "test.h"
#include "usctest.h"
#include "linux_syscall_numbers.h"

#define TEST_CASES 5
#define BUFF_SIZE 256
#define MYRETCODE -999
#ifndef AT_FDCWD
#define AT_FDCWD -100
#endif
#ifndef AT_REMOVEDIR
#define AT_REMOVEDIR 0x200
#endif

void setup();
void cleanup();
void setup_every_copy();

char *TCID = "readlinkat01";	/* Test program identifier.    */
int TST_TOTAL = TEST_CASES;	/* Total number of test cases. */
char pathname[256];
char dpathname[256];
char testfile[256];
char dtestfile[256];
char testfile2[256];
char dtestfile2[256];
char testfile3[256];
char dtestfile3[256];
int dirfd, fd, ret;
int fds[TEST_CASES];
char *filenames[TEST_CASES];
int expected_errno[TEST_CASES] = { 0, 0, ENOTDIR, EBADF, 0 };
char expected_buff[TEST_CASES][256];
char buffer[BUFF_SIZE];

int myreadlinkat(int dirfd, const char *filename, char *buffer, size_t bufsize)
{
	return syscall(__NR_readlinkat, dirfd, filename, buffer, bufsize);
}

int main(int ac, char **av)
{
	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */
	int i;

	/* Disable test if the version of the kernel is less than 2.6.16 */
	if ((tst_kvercmp(2, 6, 16)) < 0) {
		tst_resm(TWARN, "This test can only run on kernels that are ");
		tst_resm(TWARN, "2.6.16 and higher");
		exit(0);
	}

	/***************************************************************
	 * parse standard options
	 ***************************************************************/
	if ((msg = parse_opts(ac, av, NULL, NULL)) != NULL)
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);

	/***************************************************************
	 * perform global setup for test
	 ***************************************************************/
	setup();

	/***************************************************************
	 * check looping state if -c option given
	 ***************************************************************/
	for (lc = 0; TEST_LOOPING(lc); lc++) {
		setup_every_copy();

		Tst_count = 0;

		/*
		 * Call readlinkat
		 */
		for (i = 0; i < TST_TOTAL; i++) {
			buffer[0] = '\0';
			TEST(myreadlinkat
			     (fds[i], filenames[i], buffer, BUFF_SIZE));

			if (TEST_RETURN >= 0) {
				buffer[TEST_RETURN] = '\0';
			}

			/* check return code */
			if (TEST_ERRNO == expected_errno[i]
			    && (strcmp(expected_buff[i], buffer) == 0)) {

				/***************************************************************
				 * only perform functional verification if flag set (-f not given)
				 ***************************************************************/
				if (STD_FUNCTIONAL_TEST) {
					/* No Verification test, yet... */
					tst_resm(TPASS,
						 "readlinkat() returned the expected  errno %d: %s",
						 TEST_ERRNO,
						 strerror(TEST_ERRNO));
				}
			} else {
				if (TEST_RETURN >= 0) {
					tst_resm(TINFO,
						 "The link readlinkat got isn't as same as the expected");
				}
				TEST_ERROR_LOG(TEST_ERRNO);
				tst_resm(TFAIL,
					 "readlinkat() Failed, errno=%d : %s",
					 TEST_ERRNO, strerror(TEST_ERRNO));
			}
		}

	}

	/***************************************************************
	 * cleanup and exit
	 ***************************************************************/
	cleanup();

	return (0);
}

void setup_every_copy()
{
	int i;
	char tmpfilename[256] = "";

	/* Initialize test dir and file names */
	sprintf(pathname, "readlinkattestdir%d", getpid());
	sprintf(dpathname, "dreadlinkattestdir%d", getpid());
	sprintf(testfile, "readlinkattestfile%d.txt", getpid());
	sprintf(dtestfile, "dreadlinkattestfile%d.txt", getpid());
	sprintf(testfile2, "readlinkattestdir%d/readlinkattestfile%d.txt",
		getpid(), getpid());
	sprintf(dtestfile2, "dreadlinkattestdir%d/dreadlinkattestfile%d.txt",
		getpid(), getpid());
	sprintf(testfile3, "/tmp/readlinkattestfile%d.txt", getpid());
	sprintf(dtestfile3, "/tmp/dreadlinkattestfile%d.txt", getpid());

	ret = mkdir(pathname, 0700);
	if (ret < 0) {
		perror("mkdir: ");
		exit(-1);
	}

	ret = mkdir(dpathname, 0700);
	if (ret < 0) {
		perror("mkdir: ");
		exit(-1);
	}

	dirfd = open(dpathname, O_DIRECTORY);
	if (dirfd < 0) {
		perror("open: ");
		exit(-1);
	}

	fd = open(testfile, O_CREAT | O_RDWR, 0600);
	if (fd < 0) {
		perror("open: ");
		exit(-1);
	}

	ret = symlink(testfile, dtestfile);
	if (ret < 0) {
		perror("symlink: ");
		exit(-1);
	}

	fd = open(testfile2, O_CREAT | O_RDWR, 0600);
	if (fd < 0) {
		perror("open: ");
		exit(-1);
	}

	tmpfilename[0] = '\0';
	strcat(strcat(tmpfilename, "../"), testfile2);
	ret = symlink(tmpfilename, dtestfile2);
	if (ret < 0) {
		perror("symlink: ");
		exit(-1);
	}

	fd = open(testfile3, O_CREAT | O_RDWR, 0600);
	if (fd < 0) {
		perror("open: ");
		exit(-1);
	}

	ret = symlink(testfile3, dtestfile3);
	if (ret < 0) {
		perror("symlink: ");
		exit(-1);
	}

	fds[0] = fds[1] = dirfd;
	fds[2] = fd;
	fds[3] = 100;
	fds[4] = AT_FDCWD;

	filenames[0] = filenames[2] = filenames[3] = filenames[4] = dtestfile;
	filenames[1] = dtestfile3;

	for (i = 0; i < TEST_CASES; i++)
		expected_buff[i][0] = '\0';

	strcat(strcat(expected_buff[0], "../"), testfile2);
	strcat(expected_buff[1], testfile3);
	strcat(expected_buff[2], "");
	strcat(expected_buff[3], "");
	strcat(expected_buff[4], testfile);
}

/***************************************************************
 * setup() - performs all ONE TIME setup for this test.
 ***************************************************************/
void setup()
{

	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;
}

/***************************************************************
 * cleanup() - performs all ONE TIME cleanup for this test at
 *             completion or premature exit.
 ***************************************************************/
void cleanup()
{
	/* Remove them */
	unlink(testfile2);
	unlink(dtestfile2);
	unlink(testfile3);
	unlink(dtestfile3);
	unlink(testfile);
	unlink(dtestfile);
	rmdir(pathname);
	rmdir(dpathname);

	/*
	 * print timing stats if that option was specified.
	 * print errno log if that option was specified.
	 */
	TEST_CLEANUP;

}