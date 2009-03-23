/*
 *
 *   Copyright (c) International Business Machines  Corp., 2001
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
 */

/*
 * NAME
 * 	read02.c
 *
 * DESCRIPTION
 * 	test 1:
 *	Does read return -1 if file descriptor is not valid, check for EBADF
 *
 *	test 2:
 *	Check if read sets EISDIR, if the fd refers to a directory
 *
 * 	test 3:
 * 	Check if read sets EFAULT, if buf is -1.
 *$
 * ALGORITHM
 * 	test 1:
 * 	Read with an invalid file descriptor, and expect an EBADF
 *
 * 	test 2:
 * 	The parameter passed to read is a directory, check if the errno is
 * 	set to EISDIR.
 *
 * 	test 3:
 * 	Pass buf = -1 as a parmeter to read, expect an EFAULT.
 *$
 * USAGE:  <for command-line>
 *  read02 [-c n] [-e] [-i n] [-I x] [-P x] [-t]
 *     where,  -c n : Run n copies concurrently.
 *             -e   : Turn on errno logging.
 *             -i n : Execute test n times.
 *             -I x : Execute test for x seconds.
 *             -P x : Pause for x seconds between iterations.
 *             -t   : Turn on syscall timing.
 *
 * HISTORY
 *	07/2001 Ported by Wayne Boyer
 *
 * RESTRICTIONS
 * 	None
 */
#define _GNU_SOURCE		/* for O_DIRECTORY */
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include "test.h"
#include "usctest.h"

void cleanup(void);
void setup(void);

char *TCID = "read02";
extern int Tst_count;

char file[BUFSIZ];
char fname[100] = "/tmp/tstfile";

int exp_enos[] = { EBADF, EISDIR, EFAULT, 0 };

int badfd = -1;
int fd2, fd3;
char buf[BUFSIZ];

struct test_case_t {
	int *fd;
	void *buf;
	int error;
} TC[] = {
	/* the file descriptor is invalid - EBADF */
	{
	&badfd, buf, EBADF},
	    /* the file descriptor is a directory - EISDIR */
	{
	&fd2, buf, EISDIR,},
#ifndef UCLINUX
	    /* Skip since uClinux does not implement memory protection */
	    /* the buffer is invalid - EFAULT */
	{
	&fd3, (void *)-1, EFAULT}
#endif
};

int TST_TOTAL = sizeof(TC) / sizeof(*TC);

char *bad_addr = 0;

int main(int ac, char **av)
{
	int i;
	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */

	/* parse standard options */
	if ((msg = parse_opts(ac, av, (option_t *) NULL, NULL)) != (char *)NULL) {
		tst_brkm(TBROK, tst_exit, "OPTION PARSING ERROR - %s", msg);
	}

	setup();

	/* set up the expected errnos */
	TEST_EXP_ENOS(exp_enos);

	/*
	 * The following loop checks looping state if -i option given
	 */
	for (lc = 0; TEST_LOOPING(lc); lc++) {
		/* reset Tst_count in case we are looping */
		Tst_count = 0;

		/* loop through the test cases */
		for (i = 0; i < TST_TOTAL; i++) {

			TEST(read(*TC[i].fd, TC[i].buf, 1));

			if (TEST_RETURN != -1) {
				tst_resm(TFAIL, "call succeeded unexpectedly");
				continue;
			}

			TEST_ERROR_LOG(TEST_ERRNO);

			if (TEST_ERRNO == TC[i].error) {
				tst_resm(TPASS, "expected failure - "
					 "errno = %d : %s", TEST_ERRNO,
					 strerror(TEST_ERRNO));
			} else {
				tst_resm(TFAIL, "unexpected error - %d : %s - "
					 "expected %d", TEST_ERRNO,
					 strerror(TEST_ERRNO), TC[i].error);
			}
		}
	}
	cleanup();
	 /*NOTREACHED*/ return 0;
}

/*
 * setup() - performs all ONE TIME setup for this test
 */
void setup(void)
{
	/* capture signals */
	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	/* create a temporary filename */
	sprintf(fname, "%s.%d", fname, getpid());

	/* Pause if that option was specified */
	TEST_PAUSE;

	if ((fd2 = open("/tmp", O_DIRECTORY)) == -1) {
		tst_brkm(TBROK, cleanup, "open of fd2 failed");
	}

	if ((fd3 = open(fname, O_RDWR | O_CREAT, 0666)) == -1) {
		tst_brkm(TBROK, cleanup, "open of fd3 (temp file) failed");
	}

	if (write(fd3, "A", 1) != 1) {
		tst_brkm(TBROK, cleanup, "can't write to fd3");
	 /*NOTREACHED*/}
	close(fd3);
	if ((fd3 = open(fname, O_RDWR | O_CREAT, 0666)) == -1) {
		tst_brkm(TBROK, cleanup, "open of fd3 (temp file) failed");
	}
#if !defined(UCLINUX)
	bad_addr = mmap(0, 1, PROT_NONE,
			MAP_PRIVATE_EXCEPT_UCLINUX | MAP_ANONYMOUS, 0, 0);
	if (bad_addr == MAP_FAILED) {
		tst_brkm(TBROK, cleanup, "mmap failed");
	}
	TC[2].buf = bad_addr;
#endif
}

/*
 * cleanup() - performs all the ONE TIME cleanup for this test at completion
 *	       or premature exit.
 */
void cleanup(void)
{
	/*
	 * print timing status if that option was specified.
	 * print errno log if that option was specified.
	 */
	TEST_CLEANUP;
	unlink(fname);
	tst_exit();
}
