/*
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
 * Test Name :	sysinfo01
 *
 * Test description
 *  Verify that sysinfo() succeeds to get the system information and fills
 *  the structure passed.
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
 *  Check return code, if system call failed (return=-1)
 *	Log the errno and Issue a FAIL message.
 *  Otherwise,
 *	if we are being called by another sysinfo test.
 *		Print the infomation that was returned for use by the calling
 *		test.
 *	otherwise,
 *		Report success.
 * Cleanup:
 *  Print errno log and/or timing stats if options given
 *  Delete the temporary directory created.
 *
 * USAGE:  <for command-line>
 *	sysinfo01 [-c n] [-i n] [-I x] [-P x] [-t]
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
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/signal.h>
#include <sys/sysinfo.h>

#include "test.h"

void setup();
void cleanup();

char *TCID = "sysinfo01";
int TST_TOTAL = 1;

int main(int ac, char **av)
{
	struct sysinfo *sys_buf;
	int lc;
	float l1, l2, l3;
	unsigned long l1_up, l2_up, l3_up;

	sys_buf = malloc(sizeof(struct sysinfo));

	tst_parse_opts(ac, av, NULL, NULL);

	setup();		/* Global setup */

	/* The following loop checks looping state if -i option given */
	for (lc = 0; TEST_LOOPING(lc); lc++) {

		/* reset tst_count in case we are looping */
		tst_count = 0;

		TEST(sysinfo(sys_buf));
		/* check return code */
		if (TEST_RETURN == -1) {
			/* To gather stats on errnos returned, log the errno */
			tst_brkm(TFAIL, cleanup, "sysinfo() Failed, errno=%d"
				 " : %s", TEST_ERRNO, strerror(TEST_ERRNO));
		} else {
			/* Test succeeded */

			/* This portion of the code generates information
			 * used by sysinfo03 to test the functionality of
			 * sysinfo.
			 */

			if (ac == 2 && !strncmp(av[1], "TEST3", 5)) {
				tst_resm(TINFO, "Generating info for "
					 "sysinfo03");
				l1 = sys_buf->loads[0] / 60000.0;
				l2 = sys_buf->loads[1] / 60000.0;
				l3 = sys_buf->loads[2] / 60000.0;
				l1_up = l1 * 100;
				l2_up = l2 * 100;
				l3_up = l3 * 100;
				sys_buf->loads[0] = sys_buf->loads[0] / 10;
				sys_buf->loads[1] = sys_buf->loads[1] / 10;
				sys_buf->loads[2] = sys_buf->loads[2] / 10;
				printf("uptime %lu\n", sys_buf->uptime);
				printf("load1 %lu\n", sys_buf->loads[0]);
				printf("load2 %lu\n", sys_buf->loads[1]);
				printf("load3 %lu\n", sys_buf->loads[2]);
				printf("l1 %lu\n", l1_up);
				printf("l2 %lu\n", l2_up);
				printf("l3 %lu\n", l3_up);
				printf("totalram %lu\n", sys_buf->totalram);
				printf("freeram  %lu\n", sys_buf->freeram);
				printf("sharedram %lu\n", sys_buf->sharedram);
				printf("bufferram %lu\n", sys_buf->bufferram);
				printf("totalswap %lu\n",
				       sys_buf->totalswap / (1024 * 1024));
				printf("freeswap %lu\n", sys_buf->freeswap);
				printf("procs %lu\n",
				       (unsigned long)sys_buf->procs);
			} else {
				tst_resm(TPASS,
					 "Test to check the return code PASSED");
			}
		}
	}

	cleanup();
	tst_exit();

}

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
