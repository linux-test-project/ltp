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
 *      openat01.c
 *
 * DESCRIPTION
 *	This test case will verify basic function of openat
 *	added by kernel 2.6.16 or up.
 *
 * USAGE:  <for command-line>
 * openat01 [-c n] [-e] [-i n] [-I x] [-P x] [-t] [-p]
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
 *      08/22/2006      Created first by Yi Yang <yyangcdl@cn.ibm.com>
 *
 *****************************************************************************/

#define _GNU_SOURCE

#include <sys/types.h>
#include <sys/stat.h>
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
#ifndef AT_FDCWD
#define AT_FDCWD -100
#endif
void setup();
void cleanup();
void setup_every_copy();

char *TCID = "openat01";	/* Test program identifier.    */
int TST_TOTAL = TEST_CASES;	/* Total number of test cases. */
char pathname[256];
char testfile[256];
char testfile2[256];
char testfile3[256];
int dirfd, fd, ret;
int fds[TEST_CASES];
char *filenames[TEST_CASES];
int expected_errno[TEST_CASES] = { 0, 0, ENOTDIR, EBADF, 0 };

int myopenat(int dirfd, const char *filename, int flags, int mode)
{
	return syscall(__NR_openat, dirfd, filename, flags, mode);
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
		 * Call openat
		 */
		for (i = 0; i < TST_TOTAL; i++) {
			TEST(myopenat
			     (fds[i], filenames[i], O_CREAT | O_WRONLY, 0600));

			/* check return code */
			if (TEST_ERRNO == expected_errno[i]) {

				/***************************************************************
				 * only perform functional verification if flag set (-f not given)
				 ***************************************************************/
				if (STD_FUNCTIONAL_TEST) {
					/* No Verification test, yet... */
					tst_resm(TPASS,
						 "openat() returned the expected errno %d: %s",
						 TEST_ERRNO,
						 strerror(TEST_ERRNO));
				}
			} else {
				TEST_ERROR_LOG(TEST_ERRNO);
				tst_resm(TFAIL,
					 "openat() Failed, errno=%d : %s",
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
	/* Initialize test dir and file names */
	sprintf(pathname, "openattestdir%d", getpid());
	sprintf(testfile, "openattestfile%d.txt", getpid());
	sprintf(testfile2, "openattestfile%d_2.txt", getpid());
	sprintf(testfile3, "/tmp/openattestfile%d.txt", getpid());

	ret = mkdir(pathname, 0600);
	if (ret < 0) {
		perror("mkdir: ");
		exit(-1);
	}

	dirfd = open(pathname, O_DIRECTORY);
	if (dirfd < 0) {
		perror("open: ");
		exit(-1);
	}

	fd = open(testfile2, O_CREAT | O_RDWR, 0600);
	if (fd < 0) {
		perror("open: ");
		exit(-1);
	}
	fds[0] = fds[1] = dirfd;
	fds[2] = fd;
	fds[3] = 100;
	fds[4] = AT_FDCWD;

	filenames[0] = filenames[3] = filenames[4] = testfile;
	filenames[1] = testfile3;
	filenames[2] = testfile2;
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
 *		completion or premature exit.
 ***************************************************************/
void cleanup()
{
	/* Remove them */
	char tmppathname[256];
	strcpy(tmppathname, pathname);
	unlink(strcat(strcat(tmppathname, "/"), testfile));
	unlink(testfile);
	strcpy(tmppathname, pathname);
	unlink(strcat(strcat(tmppathname, "/"), testfile2));
	unlink(testfile2);
	unlink(testfile3);
	rmdir(pathname);

	/*
	 * print timing stats if that option was specified.
	 * print errno log if that option was specified.
	 */
	TEST_CLEANUP;

}