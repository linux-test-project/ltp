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
 *	sched_setscheduler01.c
 *
 * DESCRIPTION
 *	Testcase to test whether sched_setscheduler(2) sets the errnos
 *	correctly.
 *
 * ALGORITHM
 *	1.	Call sched_setscheduler with an invalid pid, and expect
 *	ESRCH to be returned.
 *	2.	Call sched_setscheduler with an invalid scheduling policy,
 *	and expect EINVAL to be returned.
 *	3.	Call sched_setscheduler with an invalid "param" address,
 *	which lies outside the address space of the process, and expect
 *	EFAULT to be returned.
 *	4.	Call sched_setscheduler with an invalid priority value
 *	in "param" and expect EINVAL to be returned
 *
 * USAGE:  <for command-line>
 *  sched_setscheduler01 [-c n] [-e] [-i n] [-I x] [-P x] [-t]
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
 *	None
 */
#include <stdio.h>
#include <errno.h>
#include <sched.h>
#include <pwd.h>
#include "test.h"
#include "usctest.h"

#define SCHED_INVALID	99
#define INVALID_PID	999999

char *TCID = "sched_setscheduler01";

struct sched_param param;
struct sched_param param1 = { 1 };
int exp_enos[] = { ESRCH, EINVAL, EFAULT, 0 };

void setup(void);
void cleanup(void);

struct test_case_t {
	pid_t pid;
	int policy;
	struct sched_param *p;
	int error;
} TC[] = {
	/* The pid is invalid - ESRCH */
	{
	INVALID_PID, SCHED_OTHER, &param, ESRCH},
	    /* The policy is invalid - EINVAL */
	{
	1, SCHED_INVALID, &param, EINVAL},
#ifndef UCLINUX
	    /* Skip since uClinux does not implement memory protection */
	    /* The param address is invalid - EFAULT */
	{
	1, SCHED_OTHER, (struct sched_param *)-1, EFAULT},
#endif
	    /* The priority value in param invalid - EINVAL */
	{
	0, SCHED_OTHER, &param1, EINVAL}
};

int TST_TOTAL = sizeof(TC) / sizeof(*TC);

int main(int ac, char **av)
{
	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */

	int i;

	/* parse standard options */
	if ((msg = parse_opts(ac, av, NULL, NULL)) != NULL) {
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);
	 }

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		/* reset Tst_count in case we are looping */
		Tst_count = 0;

		setup();

		/* set up the expected errnos */
		TEST_EXP_ENOS(exp_enos);

		/* loop through the test cases */
		for (i = 0; i < TST_TOTAL; i++) {

			TEST(sched_setscheduler(TC[i].pid, TC[i].policy,
						TC[i].p));

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

	tst_exit();

}

/*
 * setup() - performs all ONE TIME setup for this test.
 */
void setup()
{

	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;
}

/*
 * cleanup() - performs all ONE TIME cleanup for this test at
 *	       completion or premature exit.
 */
void cleanup()
{
	/*
	 * print timing stats if that option was specified.
	 * print errno log if that option was specified.
	 */
	TEST_CLEANUP;

}