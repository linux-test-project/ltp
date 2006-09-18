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
#include "linux_syscall_numbers.h"

#define TEST_CASES 5
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
static int mysymlinkat_test(int testno, const char *ofn,
			    int nfd, const char *nfn);

char *TCID = "symlinkat01";	/* Test program identifier.    */
int TST_TOTAL = TEST_CASES;	/* Total number of test cases. */
extern int Tst_count;		/* Test Case counter for tst_* routines */
char pathname[256] = "";
char dpathname[256] = "";
char testfile[256] = "";
char dtestfile[256] = "";
char testfile2[256] = "";
char dtestfile2[256] = "";
char testfile3[256] = "";
char dtestfile3[256] = "";
int newdirfd, fd, ret;
int newfds[TEST_CASES];
char *oldfilenames[TEST_CASES], *newfilenames[TEST_CASES];
int expected_errno[TEST_CASES] = { 0, 0, ENOTDIR, EBADF, 0 };
char buffer[VERIFICATION_BLOCK_SIZE];
char tmpfilename[256] = "";

int mysymlinkat(const char *oldfilename, int newdirfd, const char *newfilename)
{
	return syscall(__NR_symlinkat, oldfilename, newdirfd, newfilename);
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
		 * Call symlinkat 
		 */
		for (i = 0; i < TST_TOTAL; i++) {
			TEST(mysymlinkat_test
			     (i, oldfilenames[i], newfds[i], newfilenames[i]));

			/* check return code */
			if (TEST_ERRNO == expected_errno[i]) {

				/***************************************************************
				 * only perform functional verification if flag set (-f not given)
				 ***************************************************************/
				if (STD_FUNCTIONAL_TEST) {
					/* No Verification test, yet... */
					tst_resm(TPASS,
						 "symlinkat() returned the expected  errno %d: %s",
						 TEST_ERRNO,
						 strerror(TEST_ERRNO));
				}
			} else {
				if ((TEST_ERRNO == ENOMSG)
				    && (TEST_RETURN == MYRETCODE)) {
					tst_resm(TINFO,
						 "The link file's content isn't as same as the original file's "
						 "although symlinkat returned 0");
				}
				TEST_ERROR_LOG(TEST_ERRNO);
				tst_resm(TFAIL,
					 "symlinkat() Failed, errno=%d : %s",
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
	tmpfilename[0] = '\0';
	sprintf(pathname, "symlinkattestdir%d", getpid());
	sprintf(dpathname, "dsymlinkattestdir%d", getpid());
	sprintf(testfile, "symlinkattestfile%d.txt", getpid());
	sprintf(dtestfile, "dsymlinkattestfile%d.txt", getpid());
	sprintf(testfile2, "symlinkattestdir%d/symlinkattestfile%d.txt",
		getpid(), getpid());
	sprintf(dtestfile2, "dsymlinkattestdir%d/dsymlinkattestfile%d.txt",
		getpid(), getpid());
	sprintf(testfile3, "/tmp/symlinkattestfile%d.txt", getpid());
	sprintf(dtestfile3, "/tmp/dsymlinkattestfile%d.txt", getpid());

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

	newfds[0] = newfds[1] = newdirfd;
	newfds[2] = fd;
	newfds[3] = 100;
	newfds[4] = AT_FDCWD;

	strcat(strcat(tmpfilename, "../"), testfile2);
	oldfilenames[0] = oldfilenames[2] = oldfilenames[3] = tmpfilename;
	oldfilenames[1] = testfile3;
	oldfilenames[4] = testfile;

	newfilenames[0] = newfilenames[2] = newfilenames[3] = newfilenames[4] =
	    dtestfile;
	newfilenames[1] = dtestfile3;
}

static int mysymlinkat_test(int testno, const char *ofn,
			    int nfd, const char *nfn)
{
	int ret, tmperrno, fd;
	char linkbuf[VERIFICATION_BLOCK_SIZE];
	char *filename = NULL;
	int len;

	ret = mysymlinkat(ofn, nfd, nfn);
	if (ret < 0) {
		return ret;
	}

	tmperrno = errno;

	if (testno == 0)
		filename = dtestfile2;
	else if (testno == 1)
		filename = dtestfile3;
	else if (testno == 4)
		filename = dtestfile;

	if (filename == NULL) {
		errno = tmperrno;
		return ret;
	} else {
		fd = open(filename, O_RDONLY);
		if (fd < 0) {
			printf("filename = %s\n", filename);
			perror("open11: ");
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
