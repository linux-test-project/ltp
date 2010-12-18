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
 *      mkdirat01.c
 *
 * DESCRIPTION
 *	This test case will verify basic function of mkdirat
 *	added by kernel 2.6.16 or up.
 *
 * USAGE:  <for command-line>
 * mkdirat01 [-c n] [-e] [-i n] [-I x] [-P x] [-t] [-p]
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

char *TCID = "mkdirat01";	/* Test program identifier.    */
int TST_TOTAL = TEST_CASES;	/* Total number of test cases. */
char testdir[256];
char testsubdir[256];
char testsubdir2[256];
char testfile[256];
int dirfd, fd, ret;
int fds[TEST_CASES];
char *dirnames[TEST_CASES];
int expected_errno[TEST_CASES] = { 0, 0, ENOTDIR, EBADF, 0 };

int mymkdirat(int dirfd, const char *dirname, int mode)
{
	return syscall(__NR_mkdirat, dirfd, dirname, mode);
}

int main(int ac, char **av)
{
	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */
	int i;

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
		 * Call mkdirnat
		 */
		for (i = 0; i < TST_TOTAL; i++) {
			TEST(mymkdirat(fds[i], dirnames[i], 0600));

			/* check return code */
			if (TEST_ERRNO == expected_errno[i]) {
				/***************************************************************
				 * only perform functional verification if flag set (-f not given)
				 ***************************************************************/
				if (STD_FUNCTIONAL_TEST) {
					/* No Verification test, yet... */
					tst_resm(TPASS,
						 "mkdirat() returned the expected errno %d: %s",
						 TEST_ERRNO,
						 strerror(TEST_ERRNO));
				}
			} else {
				TEST_ERROR_LOG(TEST_ERRNO);
				tst_resm(TFAIL,
					 "mkdirat() Failed, errno=%d : %s",
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
	sprintf(testdir, "mkdirattestdir%d", getpid());
	sprintf(testsubdir, "mkdirattestsubdir%d", getpid());
	sprintf(testsubdir2, "/tmp/mkdirattestsubdir%d_2", getpid());
	sprintf(testfile, "mkdirattestfile%d.txt", getpid());

	ret = mkdir(testdir, 0600);
	if (ret < 0) {
		perror("mkdir: ");
		exit(-1);
	}

	dirfd = open(testdir, O_DIRECTORY);
	if (dirfd < 0) {
		perror("open: ");
		exit(-1);
	}

	fd = open(testfile, O_CREAT | O_RDWR, 0600);
	if (fd < 0) {
		perror("open: ");
		exit(-1);
	}
	fds[0] = fds[1] = dirfd;
	fds[2] = fd;
	fds[3] = 100;
	fds[4] = AT_FDCWD;

	dirnames[0] = dirnames[2] = dirnames[3] = dirnames[4] = testsubdir;
	dirnames[1] = testsubdir2;
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
	char tmpdirname[256];
	strcpy(tmpdirname, testdir);
	rmdir(strcat(strcat(tmpdirname, "/"), testsubdir));
	rmdir(testsubdir);
	rmdir(testsubdir2);
	unlink(testfile);
	rmdir(testdir);

	/*
	 * print timing stats if that option was specified.
	 * print errno log if that option was specified.
	 */
	TEST_CLEANUP;

}