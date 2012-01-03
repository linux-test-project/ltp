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
 *      symlinkat01.c
 *
 * DESCRIPTION
 *	This test case will verify basic function of symlinkat
 *	added by kernel 2.6.16 or up.
 *
 * USAGE:  <for command-line>
 * symlinkat01 [-c n] [-e] [-i n] [-I x] [-P x] [-t] [-p]
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
 *      08/25/2006      Created first by Yi Yang <yyangcdl@cn.ibm.com>
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
#include "rmobj.h"
#include "linux_syscall_numbers.h"

#define MYRETCODE -999
#ifndef AT_FDCWD
#define AT_FDCWD -100
#endif

struct test_struct;
static void setup();
static void cleanup();
static void setup_every_copy();
static void mysymlinkat_test(struct test_struct* desc);

#define TEST_DIR1 "olddir"
#define TEST_DIR2 "newdir"
#define TEST_DIR3 "deldir"
#define TEST_FILE1 "oldfile"
#define TEST_FILE2 "newfile"
#define TEST_FIFO "fifo"

static char dpathname[256] = "%s/"TEST_DIR2"/"TEST_FILE1;
static int olddirfd, newdirfd = -1, cwd_fd = AT_FDCWD, stdinfd = 0, crapfd = -1, deldirfd;

struct test_struct {
	const char* oldfn;
	int* newfd;
	const char* newfn;
	const char* referencefn1;
	const char* referencefn2;
	int expected_errno;
} test_desc[]= {
	/* relative paths */
	{ "../"TEST_DIR1"/"TEST_FILE1, &newdirfd, TEST_FILE1, TEST_DIR1"/"TEST_FILE1, TEST_DIR2"/"TEST_FILE1, 0 },
	/* abs path at dst */
	{ "../"TEST_DIR1"/"TEST_FILE1, &newdirfd, dpathname, TEST_DIR1"/"TEST_FILE1, TEST_DIR2"/"TEST_FILE1, 0},

	/* relative paths to cwd */
	{ "../"TEST_DIR1"/"TEST_FILE1, &cwd_fd, TEST_DIR2"/"TEST_FILE1, TEST_DIR1"/"TEST_FILE1, TEST_DIR2"/"TEST_FILE1, 0 },
	/* abs path */
	{ "../"TEST_DIR1"/"TEST_FILE1, &cwd_fd, dpathname, TEST_DIR1"/"TEST_FILE1, TEST_DIR2"/"TEST_FILE1, 0},

	/* relative paths to invalid */
	{ "../"TEST_DIR1"/"TEST_FILE1, &stdinfd, TEST_DIR2"/"TEST_FILE1, 0, 0, ENOTDIR },
	/* abs path at dst */
	{ "../"TEST_DIR1"/"TEST_FILE1, &stdinfd, dpathname, TEST_DIR1"/"TEST_FILE1, TEST_DIR2"/"TEST_FILE1, 0},

	/* relative paths to crap */
	{ "../"TEST_DIR1"/"TEST_FILE1, &crapfd, TEST_DIR2"/"TEST_FILE1, 0, 0, EBADF },
	/* abs path at dst */
	{ "../"TEST_DIR1"/"TEST_FILE1, &crapfd, dpathname, TEST_DIR1"/"TEST_FILE1, TEST_DIR2"/"TEST_FILE1, 0},

	/* relative paths to deleted */
	{ "../"TEST_DIR1"/"TEST_FILE1, &deldirfd, TEST_DIR2"/"TEST_FILE1, 0, 0, ENOENT },
	/* abs path at dst */
	{ "../"TEST_DIR1"/"TEST_FILE1, &deldirfd, dpathname, TEST_DIR1"/"TEST_FILE1, TEST_DIR2"/"TEST_FILE1, 0},

	/* fifo link */
	/*	{ TEST_FIFO, &newdirfd, TEST_FILE1, TEST_DIR1"/"TEST_FIFO, TEST_DIR2"/"TEST_FILE1, 0 },*/
};

char *TCID = "symlinkat01";	/* Test program identifier.    */
int TST_TOTAL = sizeof(test_desc) / sizeof(*test_desc);	/* Total number of test cases. */

#define SUCCEED_OR_DIE(syscall, message, ...)														\
	(errno = 0,																														\
		({int ret=syscall(__VA_ARGS__);																			\
			if (ret==-1)																												\
				tst_brkm(TBROK, cleanup, message, __VA_ARGS__, strerror(errno)); \
			ret;}))

static int mysymlinkat(const char *oldfilename,
	     int newdirfd, const char *newfilename)
{
	return syscall(__NR_symlinkat, oldfilename, newdirfd,
		       newfilename);
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

		Tst_count = 0;

		/*
		 * Call symlinkat
		 */
		for (i = 0; i < TST_TOTAL; i++) {
			setup_every_copy();
			mysymlinkat_test(&test_desc[i]);

		}

	}

	/***************************************************************
	 * cleanup and exit
	 ***************************************************************/
	cleanup();

	return (0);
}

static void setup_every_copy()
{
	close(newdirfd);
	rmobj(TEST_DIR2, NULL);

	SUCCEED_OR_DIE(mkdir, "mkdir(%s, %o) failed: %s", TEST_DIR2, 0700);
	newdirfd = SUCCEED_OR_DIE(open, "open(%s, 0x%x) failed: %s", TEST_DIR2, O_DIRECTORY);
}

static void mysymlinkat_test(struct test_struct* desc)
{
	int fd;

	TEST(mysymlinkat(desc->oldfn, *desc->newfd, desc->newfn));

	/* check return code */
	if (TEST_ERRNO == desc->expected_errno) {

		/***************************************************************
		 * only perform functional verification if flag set (-f not given)
		 ***************************************************************/
		if (STD_FUNCTIONAL_TEST) {
			/* No Verification test, yet... */

			if (TEST_RETURN == 0 && desc->referencefn1 != NULL) {
				int tnum=rand(), vnum=~tnum;
				int len;
				fd = SUCCEED_OR_DIE(open, "open(%s, 0x%x) failed: %s", desc->referencefn1, O_RDWR);
				if ((len=write(fd, &tnum, sizeof(tnum))) != sizeof(tnum))
					tst_brkm(TBROK, cleanup, "write() failed: expected %zu, returned %d; error: %s", sizeof(tnum), len, strerror(errno));
				SUCCEED_OR_DIE(close, "close(%d) failed: %s", fd);

				fd = SUCCEED_OR_DIE(open, "open(%s, 0x%x) failed: %s", desc->referencefn2, O_RDONLY);
				if ((len=read(fd, &vnum, sizeof(vnum))) != sizeof(tnum))
					tst_brkm(TBROK, cleanup, "read() failed: expected %zu, returned %d; error: %s", sizeof(vnum), len, strerror(errno));
				SUCCEED_OR_DIE(close, "close(%d) failed: %s", fd);

				if (tnum == vnum)
					tst_resm(TPASS, "Test passed");
				else
					tst_resm(TFAIL,
									 "The link file's content isn't as same as the original file's "
									 "although symlinkat returned 0");
			}
			else
				tst_resm(TPASS,
								 "symlinkat() returned the expected  errno %d: %s",
								 TEST_ERRNO,
								 strerror(TEST_ERRNO));
		} else
			tst_resm(TPASS, "Test passed");
	} else {
		TEST_ERROR_LOG(TEST_ERRNO);
		tst_resm(TFAIL,
						 TEST_RETURN == 0 ? "symlinkat() surprisingly succeeded" : "symlinkat() Failed, errno=%d : %s",
						 TEST_ERRNO, strerror(TEST_ERRNO));
	}
}

/***************************************************************
 * setup() - performs all ONE TIME setup for this test.
 ***************************************************************/
static void setup()
{
	char *tmp;

	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	tst_tmpdir();

	SUCCEED_OR_DIE(mkdir, "mkdir(%s, %o) failed: %s", TEST_DIR1, 0700);
	SUCCEED_OR_DIE(mkdir, "mkdir(%s, %o) failed: %s", TEST_DIR3, 0700);
	olddirfd = SUCCEED_OR_DIE(open, "open(%s, 0x%x) failed: %s", TEST_DIR1, O_DIRECTORY);
	deldirfd = SUCCEED_OR_DIE(open, "open(%s, 0x%x) failed: %s", TEST_DIR3, O_DIRECTORY);
	SUCCEED_OR_DIE(rmdir, "rmdir(%s) failed: %s", TEST_DIR3);
	SUCCEED_OR_DIE(close, "close(%d) failed: %s",
								 SUCCEED_OR_DIE(open, "open(%s, 0x%x, %o) failed: %s", TEST_DIR1"/"TEST_FILE1, O_CREAT | O_EXCL, 0600));

	/* gratuitous memory leak here */
	tmp = strdup(dpathname);
	snprintf(dpathname, sizeof(dpathname), tmp, get_current_dir_name());

	TEST_PAUSE;
}

/***************************************************************
 * cleanup() - performs all ONE TIME cleanup for this test at
 *             completion or premature exit.
 ***************************************************************/
static void cleanup()
{
	tst_rmdir();
	/*
	 * print timing stats if that option was specified.
	 * print errno log if that option was specified.
	 */
	TEST_CLEANUP;

}
