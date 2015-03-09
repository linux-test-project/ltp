/*
 *
 *   Copyright (c) Red Hat Inc., 2007
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
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

/*
 * NAME
 *	posix_fadvise02.c
 *
 * DESCRIPTION
 *	Check the value that posix_fadvise returns for wrong file descriptor.
 *
 * USAGE
 *	posix_fadvise02
 *
 * HISTORY
 *	11/2007 Initial version by Masatake YAMATO <yamato@redhat.com>
 *
 * RESTRICTIONS
 *	None
 */

#define _XOPEN_SOURCE 600
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include "test.h"

#include "linux_syscall_numbers.h"
#ifndef _FILE_OFFSET_BITS
#define _FILE_OFFSET_BITS 32
#endif

#ifndef __NR_fadvise64
#define __NR_fadvise64 0
#endif

void setup();
void cleanup();

TCID_DEFINE(posix_fadvise02);

#define WRONG_FD       42	/* The number has no meaning.
				   Just used as something wrong fd */

struct test_case_t {
	int fd;
	off_t offset;
	off_t len;
	int advice;
	int error;
} TC[] = {
	{
	WRONG_FD, 0, 0, POSIX_FADV_NORMAL, EBADF}, {
	WRONG_FD, 0, 0, POSIX_FADV_SEQUENTIAL, EBADF}, {
	WRONG_FD, 0, 0, POSIX_FADV_RANDOM, EBADF}, {
	WRONG_FD, 0, 0, POSIX_FADV_NOREUSE, EBADF}, {
	WRONG_FD, 0, 0, POSIX_FADV_WILLNEED, EBADF}, {
WRONG_FD, 0, 0, POSIX_FADV_DONTNEED, EBADF},};

int TST_TOTAL = sizeof(TC) / sizeof(TC[0]);

int main(int ac, char **av)
{
	int lc;
	int i;

	/* Check this system has fadvise64 system which is used
	   in posix_fadvise. */
	if ((_FILE_OFFSET_BITS != 64) && (__NR_fadvise64 == 0)) {
		tst_resm(TWARN,
			 "This test can only run on kernels that implements ");
		tst_resm(TWARN, "fadvise64 which is used from posix_fadvise");
		exit(0);
	}

	/*
	 * parse standard options
	 */
	tst_parse_opts(ac, av, NULL, NULL);

	/*
	 * perform global setup for test
	 */
	setup();

	/*
	 * check looping state if -i option given on the command line
	 */
	for (lc = 0; TEST_LOOPING(lc); lc++) {

		tst_count = 0;

		/* loop through the test cases */
		for (i = 0; i < TST_TOTAL; i++) {

			TEST(posix_fadvise
			     (TC[i].fd, TC[i].offset, TC[i].len, TC[i].advice));

			if (TEST_RETURN == 0) {
				tst_resm(TFAIL, "call succeeded unexpectedly");
				continue;
			}

			/* Man page says:
			   "On error, an error number is returned." */
			if (TEST_RETURN == TC[i].error) {
				tst_resm(TPASS, "expected failure - "
					 "returned value = %ld : %s",
					 TEST_RETURN, strerror(TEST_RETURN));
			} else {
				tst_resm(TFAIL,
					 "unexpected returnd value - %ld : %s - "
					 "expected %d", TEST_RETURN,
					 strerror(TEST_RETURN), TC[i].error);
			}
		}
	}

	/*
	 * cleanup and exit
	 */
	cleanup();

	tst_exit();
}

/*
 * setup() - performs all ONE TIME setup for this test.
 */
void setup(void)
{

	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;

	/* Make WRONG_FD really wrong. */
retry:
	errno = 0;
	if (close(WRONG_FD) != 0) {
		if (errno == EBADF) ;	/* Good. Do nothing. */
		if (errno == EINTR)
			goto retry;
		else if (errno == EIO)
			tst_brkm(TBROK, cleanup,
				 "Unable to close a file descriptor(%d): %s\n",
				 WRONG_FD, strerror(EIO));
	}

}

/*
 * cleanup() - performs all ONE TIME cleanup for this test at
 *		completion or premature exit.
 */
void cleanup(void)
{

}
