/*
 * Copyright (c) Wipro Technologies Ltd, 2002.  All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it would be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 */
/**********************************************************
 *
 *    TEST IDENTIFIER   : flock02
 *
 *    EXECUTED BY       : anyone
 *
 *    TEST TITLE        : Error condition test for flock(2)
 *
 *    TEST CASE TOTAL   : 3
 *
 *    AUTHOR            : Vatsal Avasthi <vatsal.avasthi@wipro.com>
 *
 *    SIGNALS
 *      Uses SIGUSR1 to pause before test if option set.
 *      (See the parse_opts(3) man page).
 *
 *    DESCRIPTION
 * 	This test verifies that
 *	1)	flock(2) returns -1 and sets error number to EBADF
 *		if the file descriptor is invalid.
 *      2)	flock(2) returns -1 and sets error number to EINVAL
 *		if the argument operation does not include LOCK_SH,LOCK_EX,LOCK_UN.$
 *	3)	flock(2) returns -1 and sets error number to EINVAL
 *		if an invalid combination of locking modes is used i.e LOCK_SH with LOCK_EX
 *
 *	Setup:
 *        Setup signal handling.
 *        Pause for SIGUSR1 if option specified.
 *        Create a temporary directory and chdir to it.
 * 	  Create a temporary file
 *
 *	Test:
 *	Loop if proper options are given.
 *		Execute system call
 *		Check return code,
 *			if system call failed (return == -1) and errno == expected_errno
 *				Issue system call fails with expected return value and error number
 *			else
 *				Issue system call failed to produce expected error.
 *
 *      Cleanup:
 *        Print errno log and/or timing stats if options given
 *	  Deletes temporary directory.
 *
 * USAGE:  <for command-line>
 *      flock02 [-c n] [-e] [-i n] [-I x] [-P x] [-t] [-h] [-f] [-p]
 *                      where,  -c n : Run n copies concurrently.
 *                              -f   : Turn off functional testing
 *    				-e   : Turn on errno logging.
 *                              -h   : Show help screen                        $
 *				-i n : Execute test n times.
 *                              -I x : Execute test for x seconds.
 *                              -p   : Pause for SIGUSR1 before starting
 *                              -P x : Pause for x seconds between iterations.
 *                              -t   : Turn on syscall timing.
 *
 ****************************************************************/

#include <sys/types.h>
#include <sys/file.h>
#include <sys/wait.h>
#include <errno.h>
#include <stdio.h>
#include "test.h"

void setup(void);
void cleanup(void);

char *TCID = "flock02";
int TST_TOTAL = 3;
char filename[100];
int fd;

int main(int argc, char **argv)
{
	int lc;

	tst_parse_opts(argc, argv, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {

		tst_count = 0;

		TEST(flock(-1, LOCK_SH));

		if (TEST_RETURN == -1 && TEST_ERRNO == EBADF)
			tst_resm(TPASS, "flock failed as expected with EBADF");
		else if (TEST_RETURN == 0)
			tst_resm(TFAIL, "flock succeeded unexpectedly");
		else
			tst_resm(TFAIL | TTERRNO, "flock failed unexpectedly");

		/* Test system call with invalid argument */
		TEST(flock(fd, LOCK_NB));

		if (TEST_RETURN == -1 && TEST_ERRNO == EINVAL)
			tst_resm(TPASS, "flock failed as expected with EINVAL");
		else if (TEST_RETURN == 0)
			tst_resm(TFAIL, "flock succeeded unexpectedly");
		else
			tst_resm(TFAIL | TTERRNO, "flock failed unexpectedly");

		/* Test system call with invalid combination of arguments */
		TEST(flock(fd, LOCK_SH | LOCK_EX));

		if (TEST_RETURN == -1 && TEST_ERRNO == EINVAL) {
			tst_resm(TPASS, "flock failed as expected with EINVAL");
			continue;	/*next loop for MTKERNEL  */
		} else if (TEST_RETURN == 0)
			tst_resm(TFAIL, "flock succeeded unexpectedly");
		else
			tst_resm(TFAIL | TTERRNO, "flock failed unexpectedly");

	}

	close(fd);

	cleanup();

	tst_exit();

}

void setup(void)
{

	tst_sig(FORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;

	tst_tmpdir();

	sprintf(filename, "flock02.%d", getpid());

	fd = creat(filename, 0666);
	if (fd < 0)
		tst_brkm(TFAIL | TERRNO, cleanup, "creat failed");
}

void cleanup(void)
{
	unlink(filename);
	tst_rmdir();

}
