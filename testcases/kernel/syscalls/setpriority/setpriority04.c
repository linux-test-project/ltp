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
 *	setpriority04.c
 *
 * DESCRIPTION
 *	setpriority04 - test for an expected failure by using an invalid
 *			process id
 *
 * ALGORITHM
 *	loop if that option was specified
 *	issue the system call with an invalid process id
 *	check the errno value
 *	  issue a PASS message if we get ESRCH - errno 3
 *	otherwise, the tests fails
 *	  issue a FAIL message
 *	  break any remaining tests
 *	  call cleanup
 *
 * USAGE:  <for command-line>
 *  setpriority04 [-c n] [-e] [-i n] [-I x] [-p x] [-t]
 *     where,  -c n : Run n copies concurrently.
 *             -e   : Turn on errno logging.
 *	       -i n : Execute test n times.
 *	       -I x : Execute test for x seconds.
 *	       -P x : Pause for x seconds between iterations.
 *	       -t   : Turn on syscall timing.
 *
 * HISTORY
 *	03/2001 - Written by Wayne Boyer
 *
 * RESTRICTIONS
 *	none
 */

#include "test.h"
#include "usctest.h"

#include <errno.h>
#include <sys/time.h>
#include <sys/resource.h>

void cleanup(void);
void setup(void);

char *TCID = "setpriority04";
int TST_TOTAL = 1;

int exp_enos[] = { ESRCH, 0 };

/* Get the max number of message queues allowed on system */
static long get_pid_max()
{
	FILE *fp;
	char buff[512];

	/* Get the max number of message queues allowed on system */
	fp = fopen("/proc/sys/kernel/pid_max", "r");
	if (fp == NULL)
		tst_brkm(TBROK, cleanup,
			 "Could not open /proc/sys/kernel/pid_max");
	if (!fgets(buff, sizeof(buff), fp))
		tst_brkm(TBROK, cleanup,
			 "Could not read /proc/sys/kernel/pid_max");
	fclose(fp);

	return atol(buff);
}

int main(int ac, char **av)
{
	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */
	int new_val = 2;
	int pid_max = get_pid_max();

	/* parse standard options */
	if ((msg = parse_opts(ac, av, NULL, NULL)) != NULL) {
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);
	}

	setup();		/* global setup */

	/* The following loop checks looping state if -i option given */

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		/* reset Tst_count in case we are looping */
		Tst_count = 0;

		/*
		 * Try to access an invalid process.
		 * This should give an ESRCH error.
		 */

		/* call the system call with the TEST() macro */
		TEST(setpriority(PRIO_PROCESS, pid_max + 1, new_val));

		if (TEST_RETURN == 0) {
			tst_resm(TFAIL, "call failed to produce expected error "
				 "- errno = %d - %s", TEST_ERRNO,
				 strerror(TEST_ERRNO));
			continue;
		}

		TEST_ERROR_LOG(TEST_ERRNO);

		switch (TEST_ERRNO) {
		case ESRCH:
			tst_resm(TPASS, "expected failure - errno = %d - %s",
				 TEST_ERRNO, strerror(TEST_ERRNO));
			break;
		default:
			tst_resm(TFAIL, "call failed to produce expected error "
				 "- errno = %d - %s", TEST_ERRNO,
				 strerror(TEST_ERRNO));
		}
	}

	cleanup();

	tst_exit();
}

/*
 * setup() - performs all the ONE TIME setup for this test.
 */
void setup(void)
{

	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	/* Set up the expected error numbers for the -e option */
	TEST_EXP_ENOS(exp_enos);

	TEST_PAUSE;
}

/*
 * cleanup() - performs all the ONE TIME cleanup for this test at completion
 *	       or premature exit.
 */
void cleanup(void)
{
	/*
	 * print timing stats if that option was specified.
	 * print errno log if that option was specified.
	 */
	TEST_CLEANUP;

}
