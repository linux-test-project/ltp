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
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

/*
 * NAME
 *	rename08
 *
 * DESCRIPTION
 *	This test will verify that rename(2) syscall failed in EFAULT
 *
 * ALGORITHM
 *	Setup:
 *		Setup signal handling.
 *		Create temporary directory.
 *		Pause for SIGUSR1 if option specified.
 *		Create a valid file to use in the rename() call.
 *
 *	Test:
 *		Loop if the proper options are given.
 *              1.  "old" is a valid file, newpath points to address
 *                   outside allocated address space
 *                  rename the "old" to the "new" file
 *                  verify rename() failed with error EFAULT
 *
 *              2.  "old" points to address outside allocated address space
 *                  ,"new" is a valid file
 *                  rename the "old" to the "new"
 *                  verify rename() failed with error EFAULT
 *
 *              3.  oldpath and newpath are all NULL
 *                  try to rename NULL to NULL
 *                  verify rename() failed with error EFAULT
 *	Cleanup:
 *		Print errno log and/or timing stats if options given
 *		Delete the temporary directory created.*
 * USAGE
 *	rename08 [-c n] [-e] [-i n] [-I x] [-P x] [-t]
 *	where,  -c n : Run n copies concurrently.
 *		-e   : Turn on errno logging.
 *		-i n : Execute test n times.
 *		-I x : Execute test for x seconds.
 *		-P x : Pause for x seconds between iterations.
 *		-t   : Turn on syscall timing.
 *
 * HISTORY
 *	07/2001 Ported by Wayne Boyer
 *
 * RESTRICTIONS
 *	None.
 */
#include <sys/types.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <errno.h>

#include "test.h"

void setup();
void cleanup();

char *TCID = "rename08";

char *bad_addr = 0;

int fd;
char fname[255];

struct test_case_t {
	char *fd;
	char *fd2;
	int error;
} TC[] = {
#if !defined(UCLINUX)
	/* "new" file is invalid - EFAULT */
	{
	fname, (char *)-1, EFAULT},
	    /* "old" file is invalid - EFAULT */
	{
	(char *)-1, fname, EFAULT},
#endif
	    /* both files are NULL - EFAULT */
	{
	NULL, NULL, EFAULT}
};

int TST_TOTAL = ARRAY_SIZE(TC);

int main(int ac, char **av)
{
	int lc;
	int i;

	/*
	 * parse standard options
	 */
	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	/*
	 * check looping state if -i option given
	 */
	for (lc = 0; TEST_LOOPING(lc); lc++) {

		tst_count = 0;

		/* loop through the test cases */
		for (i = 0; i < TST_TOTAL; i++) {

			TEST(rename(TC[i].fd, TC[i].fd2));

			if (TEST_RETURN != -1) {
				tst_resm(TFAIL, "call succeeded unexpectedly");
				continue;
			}

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
	tst_exit();

}

/*
 * setup() - performs all ONE TIME setup for this test.
 */
void setup(void)
{

	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;

	/* Create a temporary directory and make it current. */
	tst_tmpdir();

	sprintf(fname, "./tfile_%d", getpid());

	SAFE_TOUCH(cleanup, fname, 0700, NULL);

#if !defined(UCLINUX)
	bad_addr = mmap(0, 1, PROT_NONE,
			MAP_PRIVATE_EXCEPT_UCLINUX | MAP_ANONYMOUS, 0, 0);
	if (bad_addr == MAP_FAILED) {
		tst_brkm(TBROK, cleanup, "mmap failed");
	}
	TC[0].fd2 = bad_addr;
	TC[1].fd = bad_addr;
#endif
}

/*
 * cleanup() - performs all ONE TIME cleanup for this test at
 *              completion or premature exit.
 */
void cleanup(void)
{

	/*
	 * Remove the temporary directory.
	 */
	tst_rmdir();
}
