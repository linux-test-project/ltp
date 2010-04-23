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
 *      linkat01.c
 *
 * DESCRIPTION
 *	This test case will verify basic function of linkat
 *	added by kernel 2.6.16 or up.
 *
 * USAGE:  <for command-line>
 * linkat01 [-c n] [-e] [-i n] [-I x] [-P x] [-t] [-p]
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
#include <unistd.h>
#include <error.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <inttypes.h>
#include <limits.h>
#include "test.h"
#include "usctest.h"
#include "rmobj.h"
#include "linux_syscall_numbers.h"

#ifndef AT_FDCWD
#define AT_FDCWD -100
#endif
#ifndef AT_SYMLINK_FOLLOW
#define AT_SYMLINK_FOLLOW 0x400
#endif

struct test_struct;
static void setup();
static void cleanup();
static void setup_every_copy();
static void mylinkat_test(struct test_struct* desc);

#define TEST_DIR1 "olddir"
#define TEST_DIR2 "newdir"
#define TEST_DIR3 "deldir"
#define TEST_FILE1 "oldfile"
#define TEST_FILE2 "newfile"
#define TEST_FIFO "fifo"

#define DPATHNAME_FMT	"%s/" TEST_DIR2 "/" TEST_FILE1
#define SPATHNAME_FMT	"%s/" TEST_DIR1 "/" TEST_FILE1

static char dpathname[PATH_MAX];
static char spathname[PATH_MAX];
static int olddirfd, newdirfd = -1, cwd_fd = AT_FDCWD, stdinfd = 0, badfd = -1, deldirfd;

struct test_struct {
	int* oldfd;
	const char* oldfn;
	int* newfd;
	const char* newfn;
	int flags;
	const char* referencefn1;
	const char* referencefn2;
	int expected_errno;
} test_desc[] = {
	/* 1. relative paths */
	{ &olddirfd, TEST_FILE1, &newdirfd, TEST_FILE1, 0,
	  TEST_DIR1"/"TEST_FILE1, TEST_DIR2"/"TEST_FILE1, 0 },
	/* 2. abs path at source */
	{ &olddirfd, spathname, &newdirfd, TEST_FILE1, 0,
	  0, 0, 0 },
	/* 3. abs path at dst */
	{ &olddirfd, TEST_FILE1, &newdirfd, dpathname, 0,
	  TEST_DIR1"/"TEST_FILE1, TEST_DIR2"/"TEST_FILE1, 0 },
	/* 4. relative paths to cwd */
	{ &cwd_fd, TEST_DIR1"/"TEST_FILE1, &newdirfd, TEST_FILE1, 0,
	  TEST_DIR1"/"TEST_FILE1, TEST_DIR2"/"TEST_FILE1, 0 },
	/* 5. relative paths to cwd */
	{ &olddirfd, TEST_FILE1, &cwd_fd, TEST_DIR2"/"TEST_FILE1, 0,
	  TEST_DIR1"/"TEST_FILE1, TEST_DIR2"/"TEST_FILE1, 0 },
	/* 6. abs path at source */
	{ &cwd_fd, spathname, &newdirfd, TEST_FILE1, 0, 0, 0, 0 },
	/* 7. abs path at dst */
	{ &olddirfd, TEST_FILE1, &cwd_fd, dpathname, 0,
	  TEST_DIR1"/"TEST_FILE1, TEST_DIR2"/"TEST_FILE1, 0 },
	/* 8. relative paths to invalid */
	{ &stdinfd, TEST_DIR1"/"TEST_FILE1, &newdirfd, TEST_FILE1, 0,
	  0, 0, ENOTDIR },
	/* 9. relative paths to invalid */
	{ &olddirfd, TEST_FILE1, &stdinfd, TEST_DIR2"/"TEST_FILE1, 0,
	  0, 0, ENOTDIR },
	/* 10. abs path at source */
	{ &stdinfd, spathname, &newdirfd, TEST_FILE1, 0,
	  0, 0, 0 },
	/* 11. abs path at dst */
	{ &olddirfd, TEST_FILE1, &stdinfd, dpathname, 0,
	  TEST_DIR1"/"TEST_FILE1, TEST_DIR2"/"TEST_FILE1, 0},
	/* 12. relative paths to bad */
	{ &badfd, TEST_DIR1"/"TEST_FILE1, &newdirfd, TEST_FILE1, 0,
	  0, 0, EBADF },
	/* 13. relative paths to bad */
	{ &olddirfd, TEST_FILE1, &badfd, TEST_DIR2"/"TEST_FILE1, 0,
	  0, 0, EBADF },
	/* 14. abs path at source */
	{ &badfd, spathname, &newdirfd, TEST_FILE1, 0,
	  0, 0, 0 },
	/* 15. abs path at dst */
	{ &olddirfd, TEST_FILE1, &badfd, dpathname, 0,
	  TEST_DIR1"/"TEST_FILE1, TEST_DIR2"/"TEST_FILE1, 0 },
	/* 16. relative paths to deleted */
	{ &deldirfd, TEST_DIR1"/"TEST_FILE1, &newdirfd, TEST_FILE1, 0,
	  0, 0, ENOENT },
	/* 17. relative paths to deleted */
	{ &olddirfd, TEST_FILE1, &deldirfd, TEST_DIR2"/"TEST_FILE1, 0,
	  0, 0, ENOENT },
	/* 18. abs path at source */
	{ &deldirfd, spathname, &newdirfd, TEST_FILE1, 0,
	  0, 0, 0 },
	/* 19. abs path at dst */
	{ &olddirfd, TEST_FILE1, &deldirfd, dpathname, 0,
	  TEST_DIR1"/"TEST_FILE1, TEST_DIR2"/"TEST_FILE1, 0},
	/* 20. x-device link */
	{ &cwd_fd, "/proc/cpuinfo", &newdirfd, TEST_FILE1, 0,
	  0, 0, EXDEV },
	/* 21. directory link */
	{ &olddirfd, ".", &newdirfd, TEST_FILE1, 0,
	  0, 0, EPERM },
	/* 22. invalid flag */
	{ &olddirfd, TEST_FILE1, &newdirfd, TEST_FILE1, 1,
	  0, 0, EINVAL },
	/* 23. fifo link */
	/* XXX (garrcoop): Removed because it hangs the overall test. Need to
	 * find a legitimate means to exercise this functionality, if in fact
	 * it's a valid testcase -- which it should be.
	 */
	/* { &olddirfd, TEST_FIFO, &newdirfd, TEST_FILE1, 0,
	  TEST_DIR1"/"TEST_FIFO, TEST_DIR2"/"TEST_FILE1, 0 } */
};

/* Test program identifier. */
char *TCID = "linkat01";
/* Total number of test cases. */
int TST_TOTAL = sizeof(test_desc) / sizeof(*test_desc);
/* Test Case counter for tst_* routines */
extern int Tst_count;

#define SUCCEED_OR_DIE(syscall, message, ...)		\
	(errno = 0,					\
	 ({int ret=syscall(__VA_ARGS__);		\
	 if (ret==-1)					\
		tst_brkm(TBROK | TERRNO, cleanup,	\
			message, __VA_ARGS__);		\
	 ret; }))

static int mylinkat(int olddirfd, const char *oldfilename, int newdirfd,
		    const char *newfilename, int flags)
{
	return syscall(__NR_linkat, olddirfd, oldfilename, newdirfd,
		       newfilename, flags);
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
	if ((msg = parse_opts(ac, av, (option_t *) NULL, NULL)) != (char *)NULL)
		tst_brkm(TBROK, cleanup, "OPTION PARSING ERROR - %s", msg);

	/***************************************************************
	 * perform global setup for test
	 ***************************************************************/
	setup();

	/***************************************************************
	 * check looping state if -c option given
	 ***************************************************************/
	for (lc = 0; TEST_LOOPING(lc); lc++) {

		/* reset Tst_count in case we are looping. */
		Tst_count = 0;

		/*
		 * Call linkat
		 */
		for (i = 0; i < TST_TOTAL; i++) {
			setup_every_copy();
			mylinkat_test(&test_desc[i]);
		}

	}			/* End for TEST_LOOPING */

	/***************************************************************
	 * cleanup and exit
	 ***************************************************************/
	cleanup();

	return (0);
}				/* End main */

static void setup_every_copy()
{
	close(newdirfd);
	rmobj(TEST_DIR2, NULL);

	SUCCEED_OR_DIE(mkdir, "mkdir(%s, %o) failed",
			TEST_DIR2, 0700);
	newdirfd = SUCCEED_OR_DIE(open, "open(%s, 0x%x) failed",
			TEST_DIR2, O_DIRECTORY);
}

static void mylinkat_test(struct test_struct* desc)
{
	int fd;

	TEST(mylinkat(*desc->oldfd, desc->oldfn, *desc->newfd, desc->newfn, desc->flags));

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
				fd = SUCCEED_OR_DIE(open,
						"open(%s, 0x%x) failed",
						desc->referencefn1, O_RDWR);
				if ((len=write(fd, &tnum, sizeof(tnum))) != sizeof(tnum)) {
					tst_brkm(TBROK | TERRNO, cleanup,
						"write() failed: expected %" PRId64
						", returned %d", sizeof(tnum), len);
				}
				SUCCEED_OR_DIE(close, "close(%d) failed", fd);

				fd = SUCCEED_OR_DIE(open,
							"open(%s, 0x%x) failed",
							desc->referencefn2,
							O_RDONLY);
				if((len=read(fd, &vnum, sizeof(vnum))) != sizeof(tnum)) {
					tst_brkm(TBROK | TERRNO, cleanup,
						"read() failed: expected %"
						PRId64 ", returned %d",
						sizeof(vnum), len);
				}
				SUCCEED_OR_DIE(close, "close(%d) failed", fd);

				if (tnum == vnum)
					tst_resm(TPASS, "Test passed");
				else {
					tst_resm(TFAIL,
						"The link file's content isn't "
						"as same as the original file's "
						"although linkat returned 0");
				}
			} else {
				tst_resm(TPASS | TTERRNO,
						"linkat() returned the "
						"expected errno");
			}
		} else
			tst_resm(TPASS, "Test passed");
	} else {
		TEST_ERROR_LOG(TEST_ERRNO);
		tst_resm(TFAIL | (TEST_RETURN == 0 ? 0 : TTERRNO),
			 "linkat() %s",
			(TEST_RETURN == 0 ?
			 "succeeded unexpectedly" : "failed"));
	}
}

/***************************************************************
 * setup() - performs all ONE TIME setup for this test.
 ***************************************************************/
void setup()
{
	char *cwd;

	/* capture signals */
	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	tst_tmpdir();

	cwd = get_current_dir_name();
	if (cwd == NULL) {
		tst_brkm(TFAIL | TERRNO, cleanup,
			"Failed to get current working directory");
	} else {

		SUCCEED_OR_DIE(mkdir, "mkdir(%s, %o) failed", TEST_DIR1, 0700);
		SUCCEED_OR_DIE(mkdir, "mkdir(%s, %o) failed", TEST_DIR3, 0700);
		olddirfd = SUCCEED_OR_DIE(open, "open(%s, 0x%x) failed",
						TEST_DIR1, O_DIRECTORY);
		deldirfd = SUCCEED_OR_DIE(open, "open(%s, 0x%x) failed",
						TEST_DIR3, O_DIRECTORY);
		SUCCEED_OR_DIE(rmdir, "rmdir(%s) failed", TEST_DIR3);
		SUCCEED_OR_DIE(close, "close(%d) failed",
				SUCCEED_OR_DIE(open, "open(%s, 0x%x, %o) "
						     "failed",
						TEST_DIR1"/"TEST_FILE1,
						O_CREAT | O_EXCL, 0600));

		SUCCEED_OR_DIE(mkfifo, "mkfifo(%s, %o) failed",
				TEST_DIR1"/"TEST_FIFO, 0600);

		snprintf(dpathname, sizeof(dpathname), DPATHNAME_FMT, cwd);
		snprintf(spathname, sizeof(spathname), SPATHNAME_FMT, cwd);

		free(cwd);

		/* Pause if that option was specified */
		TEST_PAUSE;

	}

} /* End setup() */

/***************************************************************
 * cleanup() - performs all ONE TIME cleanup for this test at
 *             completion or premature exit.
 ***************************************************************/
static void cleanup(void)
{
	tst_rmdir();
	/*
	 * print timing stats if that option was specified.
	 * print errno log if that option was specified.
	 */
	TEST_CLEANUP;

	/* exit with return code appropriate for results */
	tst_exit();
}				/* End cleanup() */
