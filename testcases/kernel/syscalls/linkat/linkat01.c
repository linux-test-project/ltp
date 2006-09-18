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
#include <error.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include "test.h"
#include "usctest.h"
#include "linux_syscall_numbers.h"

#define TEST_CASES 6
#define VERIFICATION_BLOCK_SIZE 1024
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
static int mylinkat_test(int testno, int ofd, const char *ofn,
			 int nfd, const char *nfn, int f);

char *TCID = "linkat01";	/* Test program identifier.    */
int TST_TOTAL = TEST_CASES;	/* Total number of test cases. */
extern int Tst_count;		/* Test Case counter for tst_* routines */
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
int expected_errno[TEST_CASES] = { 0, 0, ENOTDIR, EBADF, EINVAL, 0 };
int flags[TEST_CASES] = { 0, 0, 0, 0, 1, 0 };
char buffer[VERIFICATION_BLOCK_SIZE];

int mylinkat(int olddirfd, const char *oldfilename,
	     int newdirfd, const char *newfilename, int flags)
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
        if((tst_kvercmp(2,6,16)) < 0)
          {
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
		setup_every_copy();

		/* reset Tst_count in case we are looping. */
		Tst_count = 0;

		/* 
		 * Call linkat 
		 */
		for (i = 0; i < TST_TOTAL; i++) {
			TEST(mylinkat_test
			     (i, oldfds[i], oldfilenames[i], newfds[i],
			      newfilenames[i], flags[i]));

			/* check return code */
			if (TEST_ERRNO == expected_errno[i]) {

				/***************************************************************
				 * only perform functional verification if flag set (-f not given)
				 ***************************************************************/
				if (STD_FUNCTIONAL_TEST) {
					/* No Verification test, yet... */
					tst_resm(TPASS,
						 "linkat() returned the expected  errno %d: %s",
						 TEST_ERRNO,
						 strerror(TEST_ERRNO));
				}
			} else {
				if ((TEST_ERRNO == ENOMSG)
				    && (TEST_RETURN == MYRETCODE)) {
					tst_resm(TINFO,
						 "The link file's content isn't as same as the original file's "
						 "although linkat returned 0");
				}
				TEST_ERROR_LOG(TEST_ERRNO);
				tst_resm(TFAIL,
					 "linkat() Failed, errno=%d : %s",
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
	int len;

	/* Initialize test dir and file names */
	sprintf(pathname, "linkattestdir%d", getpid());
	sprintf(dpathname, "dlinkattestdir%d", getpid());
	sprintf(testfile, "linkattestfile%d.txt", getpid());
	sprintf(dtestfile, "dlinkattestfile%d.txt", getpid());
	sprintf(testfile2, "linkattestdir%d/linkattestfile%d.txt", getpid(),
		getpid());
	sprintf(dtestfile2, "dlinkattestdir%d/dlinkattestfile%d.txt", getpid(),
		getpid());
	sprintf(testfile3, "/tmp/linkattestfile%d.txt", getpid());
	sprintf(dtestfile3, "/tmp/dlinkattestfile%d.txt", getpid());

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

	len = write(fd, buffer, VERIFICATION_BLOCK_SIZE);
	if (len < VERIFICATION_BLOCK_SIZE) {
		perror("write: ");
		exit(-1);
	}

	fd = open(testfile2, O_CREAT | O_RDWR, 0600);
	if (fd < 0) {
		perror("open: ");
		exit(-1);
	}

	len = write(fd, buffer, VERIFICATION_BLOCK_SIZE);
	if (len < VERIFICATION_BLOCK_SIZE) {
		perror("write: ");
		exit(-1);
	}

	fd = open(testfile3, O_CREAT | O_RDWR, 0600);
	if (fd < 0) {
		perror("open: ");
		exit(-1);
	}

	len = write(fd, buffer, VERIFICATION_BLOCK_SIZE);
	if (len < VERIFICATION_BLOCK_SIZE) {
		perror("write: ");
		exit(-1);
	}

	oldfds[0] = oldfds[1] = oldfds[4] = olddirfd;
	oldfds[2] = fd;
	oldfds[3] = 100;
	oldfds[5] = AT_FDCWD;

	newfds[0] = newfds[1] = newfds[4] = newdirfd;
	newfds[2] = fd;
	newfds[3] = 100;
	newfds[5] = AT_FDCWD;

	oldfilenames[0] = oldfilenames[2] = oldfilenames[3] = oldfilenames[4] =
	    oldfilenames[5] = testfile;
	oldfilenames[1] = testfile3;

	newfilenames[0] = newfilenames[2] = newfilenames[3] = newfilenames[4] =
	    newfilenames[5] = dtestfile;
	newfilenames[1] = dtestfile3;
}

static int mylinkat_test(int testno, int ofd, const char *ofn,
			 int nfd, const char *nfn, int f)
{
	int ret, tmperrno, fd;
	char linkbuf[VERIFICATION_BLOCK_SIZE];
	char *filename = NULL;
	int len;

	ret = mylinkat(ofd, ofn, nfd, nfn, f);
	if (ret < 0) {
		return ret;
	}

	tmperrno = errno;

	if (testno == 0)
		filename = dtestfile2;
	else if (testno == 1)
		filename = dtestfile3;
	else if (testno == 5)
		filename = dtestfile;

	if (filename == NULL) {
		errno = tmperrno;
		return ret;
	} else {
		fd = open(filename, O_RDONLY);
		if (fd < 0) {
			perror("open: ");
			exit(-1);
		}
		len = read(fd, linkbuf, VERIFICATION_BLOCK_SIZE);
		if (len < 0) {
			perror("read: ");
			exit(-1);
		}
		if (memcmp(buffer, linkbuf, VERIFICATION_BLOCK_SIZE) != 0) {
			errno = ENOMSG;
			return MYRETCODE;
		}
		errno = tmperrno;
		return ret;
	}
}

/***************************************************************
 * setup() - performs all ONE TIME setup for this test.
 ***************************************************************/
void setup()
{
	/* Initilize buffer */
	int i;

	for (i = 0; i < VERIFICATION_BLOCK_SIZE; i++) {
		buffer[i] = i & 0xff;
	}

	/* capture signals */
	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	/* Pause if that option was specified */
	TEST_PAUSE;
}				/* End setup() */

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

	/* exit with return code appropriate for results */
	tst_exit();
}				/* End cleanup() */
