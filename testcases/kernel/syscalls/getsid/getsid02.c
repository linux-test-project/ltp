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
 *	getsid02.c
 *
 * DESCRIPTION
 *	getsid02 - call getsid() with an invalid PID to produce a failure
 *
 * CALLS
 *	getsid()
 *
 * ALGORITHM
 *	loop if that option was specified
 *	issue the system call with an illegal PID value
 *	check the errno value
 *	  issue a PASS message if we get ESRCH
 *	otherwise, the tests fails
 *	  issue a FAIL message
 *	  break any remaining tests
 *	  call cleanup
 *
 * USAGE:  <for command-line>
 *  getsid02 [-c n] [-e] [-i n] [-I x] [-p x] [-t]
 *     where,  -c n : Run n copies concurrently.
 *             -e   : Turn on errno logging.
 *	       -i n : Execute test n times.
 *	       -I x : Execute test for x seconds.
 *	       -P x : Pause for x seconds between iterations.
 *	       -t   : Turn on syscall timing.
 *
 * HISTORY
 *	07/2001 Ported by Wayne Boyer
 *
 * RESTRICTIONS
 *	none
 */
#define _GNU_SOURCE 1

/*
 * When attempting to build on SUSE 10, the make fails trying to compile
 * because CONFIG_BASE_SMALL is undefined.
 */
#ifndef CONFIG_BASE_SMALL
#define CONFIG_BASE_SMALL 0
#endif

#include "test.h"
#include "usctest.h"

#include <errno.h>

/*
 * See the Makefile for comments about the following preprocessor code.
 */

#ifdef _LTP_TASKS_H
#include <linux/tasks.h>	/* for PID_MAX - old */
#endif

/*
 * This is a workaround for ppc64 kernels that do not have PID_MAX defined.
 */
#if defined(__powerpc__) || defined(__powerpc64__)
#define PID_MAX 0x8000
#endif

void cleanup(void);
void setup(void);

char *TCID = "getsid02";
int TST_TOTAL = 1;
int pid_max = 32768;		/* Default value for PID_MAX  */

int exp_enos[] = { ESRCH, 0 };	/* 0 terminated list of expected errnos */

int main(int ac, char **av)
{
	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */

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
		 * call the system call with the TEST() macro
		 * with an illegal PID value
		 */

		TEST(getsid(pid_max + 1));

		if (TEST_RETURN == 0) {
			tst_resm(TFAIL, "call succeed when failure expected");
			continue;
		}

		TEST_ERROR_LOG(TEST_ERRNO);

		switch (TEST_ERRNO) {
		case ESRCH:
			tst_resm(TPASS, "expected failure - errno = %d - %s",
				 TEST_ERRNO, strerror(TEST_ERRNO));
			break;
		default:
			tst_resm(TFAIL, "call failed to produce "
				 "expected error - errno = %d - %s",
				 TEST_ERRNO, strerror(TEST_ERRNO));
			break;
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
#if !defined PID_MAX_DEFAULT && !defined PID_MAX
	FILE *fp;
#endif

#ifdef PID_MAX_DEFAULT
	pid_max = PID_MAX_DEFAULT;
#elif defined(PID_MAX)
	pid_max = PID_MAX;
#else

	if ((fp = fopen("/proc/sys/kernel/pid_max", "r")) != NULL) {
		fscanf(fp, "%d", &pid_max);
		fclose(fp);
	} else {
		tst_resm(TFAIL, "Cannot open /proc/sys/kernel/pid_max");
		exit(0);
	}
#endif

	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	/* Set up the expected error numbers for -e option */
	TEST_EXP_ENOS(exp_enos);

	TEST_PAUSE;
}

/*
 * cleanup() - performs all the ONE TIME cleanup for this test at completion
 * 	       or premature exit.
 */
void cleanup(void)
{
	/*
	 * print timing stats if that option was specified.
	 * print errno log if that option was specified.
	 */
	TEST_CLEANUP;

}