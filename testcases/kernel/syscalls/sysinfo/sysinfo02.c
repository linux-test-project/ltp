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
 * Test Name :	sysinfo02
 *
 * Test description
 *  Verify that sysinfo() returns the correct error for an invalid address structure.
 *
 * Expected Result :
 *  sysinfo() returns value 0 on success and the sysinfo structure should
 *  be filled with the system information.
 *
 * Algorithm:
 *  Setup :
 *   Setup for signal handling.
 *   Create temporary directory.
 *   Pause for SIGUSR1 if option specified.
 * Test:
 *  Loop if the proper option is given.
 *  Execute the system call.
 *  Pass an invalid address to the structure.
 *  Check return code, if system call failed (return=-1)
 *	Test case passed, Issue functionality pass message
 *  Otherwise,
 *	Issue Functionality-Fail message.
 * Cleanup:
 *  Print errno log and/or timing stats if options given
 *  Delete the temporary directory created.
 *
 * USAGE:  <for command-line>
 *	sysinfo02 [-c n] [-i n] [-I x] [-P x] [-t]
 *	where,  -c n : Run n copies concurrently.
 *		-i n : Execute test n times.
 *		-I x : Execute test for x seconds.
 *		-P x : Pause for x seconds between iterations.
 *		-t   : Turn on syscall timing.
 * History
 *	07/2001 John George
 *		-Ported
 *
 * Restrictions:
 *  None
 *
 */

#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/signal.h>
#include <sys/sysinfo.h>
#include <stdint.h>

#include "test.h"

#define INVALID_ADDRESS ((uintptr_t)-1)

void setup();
void cleanup();

char *TCID = "sysinfo02";
int TST_TOTAL = 1;

#if !defined(UCLINUX)

int main(int ac, char **av)
{
	struct sysinfo *sysinfo_buf;
	int lc;

	sysinfo_buf = (void *)INVALID_ADDRESS;

	tst_parse_opts(ac, av, NULL, NULL);

	setup();		/* Global setup */

	/* The following loop checks looping state if -i option given */
	for (lc = 0; TEST_LOOPING(lc); lc++) {

		/* reset tst_count in case we are looping */
		tst_count = 0;

		TEST(sysinfo(sysinfo_buf));
		/* check return code */
		if (TEST_RETURN != 0 && TEST_ERRNO == EFAULT) {
			/* Test succeeded as it was supposed to return -1 */
			tst_resm(TPASS,
				 "Test to check the error code %d PASSED",
				 TEST_ERRNO);
		} else {
			/* Test Failed */
			tst_brkm(TFAIL, cleanup, "sysinfo() Failed, Expected -1"
				 "returned %d/n", TEST_ERRNO);
		}
	}
	cleanup();
	tst_exit();

}

#else

int main(void)
{
	tst_resm(TINFO, "test is not available on uClinux");
	tst_exit();
}

#endif /* if !defined(UCLINUX) */

/*
 * setup()
 *	performs one time setup
 *
 */
void setup(void)
{

	tst_sig(FORK, DEF_HANDLER, cleanup);

	umask(0);

	TEST_PAUSE;
}

/*
 * cleanup()
 *
 */
void cleanup(void)
{
}
