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
 *	setrlimit02.c
 *
 * DESCRIPTION
 *	Testcase to test the different errnos set by setrlimit(2) system call.
 *
 * USAGE:  <for command-line>
 *  setrlimit02 [-c n] [-e] [-i n] [-I x] [-P x] [-t]
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
 *	NONE
 */
#include <sys/time.h>
#include <sys/resource.h>
#include <unistd.h>
#include <errno.h>
#include <pwd.h>
#include "test.h"
#include "usctest.h"

char *TCID = "setrlimit02";

char nobody_uid[] = "nobody";
struct passwd *ltpuser;

struct rlimit rlim;

void setup();
void cleanup();

int exp_enos[] = { EFAULT, EINVAL, EPERM, 0 };

struct test_case_t {
	int resource;
	struct rlimit *rlim;
	int error;
} TC[] = {
#if !defined(UCLINUX)
	/* rlim points outside the process address space - EFAULT */
	{
	RLIMIT_NOFILE, (void *)-1, EFAULT},
#endif
	    /* the resource is invalid - EINVAL */
	{
	-1, &rlim, EINVAL},
	    /* a non-root user attemps to increase the rlim_max value - EPERM */
	{
	RLIMIT_NOFILE, &rlim, EPERM}
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

	setup();

	/* set up the expected errnos */
	TEST_EXP_ENOS(exp_enos);

	for (lc = 0; TEST_LOOPING(lc); lc++) {

		Tst_count = 0;

		/* loop through the test cases */
		for (i = 0; i < TST_TOTAL; i++) {

			TEST(setrlimit(TC[i].resource, TC[i].rlim));

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

	/* Switch to nobody user for correct error code collection */
	if (geteuid() != 0) {
		tst_brkm(TBROK, NULL, "Test must be run as root");
	}
	ltpuser = getpwnam(nobody_uid);
	if (setuid(ltpuser->pw_uid) == -1) {
		tst_resm(TINFO, "setuid failed to "
			 "to set the effective uid to %d", ltpuser->pw_uid);
		perror("setuid");
	}

	/* set an illegal value for a non-root user - test #3 - EPERM */
	getrlimit(RLIMIT_NOFILE, &rlim);
	rlim.rlim_max++;
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