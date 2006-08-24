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
 *      futimesat01.c
 *
 * DESCRIPTION
 *	This test case will verify basic function of futimesat
 *	added by kernel 2.6.16 or up.
 *
 * USAGE:  <for command-line>
 * futimesat01 [-c n] [-e] [-i n] [-I x] [-P x] [-t] [-p]
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

#if defined(__powerpc__) || defined(__powerpc64__)
#define __NR_openat             286
#elif defined(__i386__)
#define __NR_openat		295
#elif defined(__sparc__) || defined(__sparc64__)
#define __NR_openat		284
#elif defined(__ia64__)
#define	__NR_openat		1281
#elif defined(__x86_64__)
#define	__NR_openat		257
#elif defined(__s390__) || defined(__s390x__)
#define	__NR_openat		288
#elif defined(__parisc__)
#define	__NR_openat		275
#else
#error unsupported arch
#endif

#define __NR_mkdirat            (__NR_openat + 1)
#define __NR_mknodat            (__NR_openat + 2)
#define __NR_fchownat           (__NR_openat + 3)
#define __NR_futimesat          (__NR_openat + 4)
#if defined(__powerpc64__) || defined(__ia64__) || defined(__x86_64__) || defined(__parisc__) || defined(__s390x__)
#define __NR_newfstatat         (__NR_openat + 5)
#else
#define __NR_fstatat64          (__NR_openat + 5)
#endif
#define __NR_unlinkat           (__NR_openat + 6)
#define __NR_renameat           (__NR_openat + 7)
#define __NR_linkat             (__NR_openat + 8)
#define __NR_symlinkat          (__NR_openat + 9)
#define __NR_readlinkat         (__NR_openat + 10)
#define __NR_fchmodat           (__NR_openat + 11)
#define __NR_faccessat          (__NR_openat + 12)

#define TEST_CASES 5
#ifndef AT_FDCWD
#define AT_FDCWD -100
#endif
void setup();
void cleanup();
void setup_every_copy();

char *TCID = "futimesat01";	/* Test program identifier.    */
int TST_TOTAL = TEST_CASES;	/* Total number of test cases. */
extern int Tst_count;		/* Test Case counter for tst_* routines */
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
	return syscall(__NR_futimesat, dirfd, filename, times);
}

int main(int ac, char **av)
{
	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */
	int i;

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
		setup_every_copy();

		/* reset Tst_count in case we are looping. */
		Tst_count = 0;

		/* 
		 * Call futimesat 
		 */
		for (i = 0; i < TST_TOTAL; i++) {
			gettimeofday(&times[0], NULL);
			gettimeofday(&times[1], NULL);
			TEST(myfutimesat(fds[i], filenames[i], times));

			/* check return code */
			if (TEST_ERRNO == expected_errno[i]) {

			/***************************************************************
			 * only perform functional verification if flag set (-f not given)
			 ***************************************************************/
				if (STD_FUNCTIONAL_TEST) {
					/* No Verification test, yet... */
					tst_resm(TPASS,
						 "futimesat() returned the expected  errno %d: %s",
						 TEST_ERRNO,
						 strerror(TEST_ERRNO));
				}
			} else {
				TEST_ERROR_LOG(TEST_ERRNO);
				tst_resm(TFAIL,
					 "futimesat() Failed, errno=%d : %s",
					 TEST_ERRNO, strerror(TEST_ERRNO));
			}
		}

	}			/* End for TEST_LOOPING */

	/***************************************************************
	 * cleanup and exit
	 ***************************************************************/
	cleanup();

	return (0);
}				/* End main */

void setup_every_copy()
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

/***************************************************************
 * setup() - performs all ONE TIME setup for this test.
 ***************************************************************/
void setup()
{
	/* capture signals */
	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	/* Pause if that option was specified */
	TEST_PAUSE;
}				/* End setup() */

/***************************************************************
 * cleanup() - performs all ONE TIME cleanup for this test at
 *		completion or premature exit.
 ***************************************************************/
void cleanup()
{
	/* Remove them */
	unlink(testfile2);
	unlink(testfile3);
	unlink(testfile);
	rmdir(pathname);

	/*
	 * print timing stats if that option was specified.
	 * print errno log if that option was specified.
	 */
	TEST_CLEANUP;

	/* exit with return code appropriate for results */
	tst_exit();
}				/* End cleanup() */
