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
 *      renameat01.c
 *
 * DESCRIPTION
 *	This test case will verify basic function of renameat
 *	added by kernel 2.6.16 or up.
 *
 * USAGE:  <for command-line>
 * renameat01 [-c n] [-e] [-i n] [-I x] [-P x] [-t] [-p]
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
#include "usctest.h"
#include "linux_syscall_numbers.h"

#define TEST_CASES 5
#ifndef AT_FDCWD
#define AT_FDCWD -100
#endif
#ifndef AT_REMOVEDIR
#define AT_REMOVEDIR 0x200
#endif

void setup();
void cleanup();
void setup_every_copy();

char *TCID = "renameat01";	/* Test program identifier.    */
int TST_TOTAL = TEST_CASES;	/* Total number of test cases. */
char pathname[256];
char dpathname[256];
char testfile[256];
char dtestfile[256];
char testfile2[256];
char dtestfile2[256];
char testfile3[256];
char dtestfile3[256];
int olddirfd, newdirfd, fd, ret;
int oldfds[TEST_CASES], newfds[TEST_CASES];
char *oldfilenames[TEST_CASES], *newfilenames[TEST_CASES];
int expected_errno[TEST_CASES] = { 0, 0, ENOTDIR, EBADF, 0 };

int myrenameat(int olddirfd, const char *oldfilename, int newdirfd,
	       const char *newfilename)
{
	return syscall(__NR_renameat, olddirfd, oldfilename, newdirfd,
		       newfilename);
}

int main(int ac, char **av)
{
	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */
	int i;

	/* Disable test if the version of the kernel is less than 2.6.16 */
	if ((tst_kvercmp(2, 6, 16)) < 0) {
		tst_brkm(TCONF, NULL, "This test can only run on kernels that are 2.6.16 and higher");
	}

	if ((msg = parse_opts(ac, av, NULL, NULL)) != NULL)
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		setup_every_copy();

		Tst_count = 0;

		/*
		 * Call renameat
		 */
		for (i = 0; i < TST_TOTAL; i++) {
			TEST(myrenameat
			     (oldfds[i], oldfilenames[i], newfds[i],
			      newfilenames[i]));

			/* check return code */
			if (TEST_ERRNO == expected_errno[i]) {

				if (STD_FUNCTIONAL_TEST) {
					/* No Verification test, yet... */
					tst_resm(TPASS|TTERRNO,
					    "renameat failed as expected");
				}
			} else {
				tst_resm(TFAIL|TTERRNO,
				    "renameat failed unexpectedly");
			}
		}

	}

	cleanup();

	tst_exit();
}

void setup_every_copy()
{
	/* Initialize test dir and file names */
	sprintf(pathname, "renameattestdir%d", getpid());
	sprintf(dpathname, "drenameattestdir%d", getpid());
	sprintf(testfile, "renameattestfile%d.txt", getpid());
	sprintf(dtestfile, "drenameattestfile%d.txt", getpid());
	sprintf(testfile2, "renameattestdir%d/renameattestfile%d.txt", getpid(),
		getpid());
	sprintf(dtestfile2, "drenameattestdir%d/drenameattestfile%d.txt",
		getpid(), getpid());
	sprintf(testfile3, "/tmp/renameattestfile%d.txt", getpid());
	sprintf(dtestfile3, "/tmp/drenameattestfile%d.txt", getpid());

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

	olddirfd = open(pathname, O_DIRECTORY);
	if (olddirfd < 0) {
		perror("open: ");
		exit(-1);
	}

	newdirfd = open(dpathname, O_DIRECTORY);
	if (newdirfd < 0) {
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

	oldfds[0] = oldfds[1] = olddirfd;
	oldfds[2] = fd;
	oldfds[3] = 100;
	oldfds[4] = AT_FDCWD;

	newfds[0] = newfds[1] = newdirfd;
	newfds[2] = fd;
	newfds[3] = 100;
	newfds[4] = AT_FDCWD;

	oldfilenames[0] = oldfilenames[2] = oldfilenames[3] = oldfilenames[4] =
	    testfile;
	oldfilenames[1] = testfile3;

	newfilenames[0] = newfilenames[2] = newfilenames[3] = newfilenames[4] =
	    dtestfile;
	newfilenames[1] = dtestfile3;
}

void setup()
{

	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;
}

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