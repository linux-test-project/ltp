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

#define SCHED_INVALID	99

char *TCID = "sched_setscheduler01";

struct sched_param param;
struct sched_param param1 = { 1 };

void setup(void);
void cleanup(void);

static pid_t unused_pid;
static pid_t init_pid = 1;
static pid_t zero_pid;

struct test_case_t {
	pid_t *pid;
	int policy;
	struct sched_param *p;
	int error;
} TC[] = {
	/* The pid is invalid - ESRCH */
	{
	&unused_pid, SCHED_OTHER, &param, ESRCH},
	    /* The policy is invalid - EINVAL */
	{
	&init_pid, SCHED_INVALID, &param, EINVAL},
#ifndef UCLINUX
	    /* Skip since uClinux does not implement memory protection */
	    /* The param address is invalid - EFAULT */
	{
	&init_pid, SCHED_OTHER, (struct sched_param *)-1, EFAULT},
#endif
	    /* The priority value in param invalid - EINVAL */
	{
	&zero_pid, SCHED_OTHER, &param1, EINVAL}
};

int TST_TOTAL = ARRAY_SIZE(TC);

int main(int ac, char **av)
{
	int lc;

	int i;

	tst_parse_opts(ac, av, NULL, NULL);

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		/* reset tst_count in case we are looping */
		tst_count = 0;

		setup();

		/* loop through the test cases */
		for (i = 0; i < TST_TOTAL; i++) {

			TEST(sched_setscheduler(*(TC[i].pid), TC[i].policy,
						TC[i].p));

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
	unused_pid = tst_get_unused_pid(cleanup);

	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;
}

/*
 * cleanup() - performs all ONE TIME cleanup for this test at
 *	       completion or premature exit.
 */
void cleanup(void)
{

}
