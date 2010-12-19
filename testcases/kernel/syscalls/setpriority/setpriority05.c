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
 *	setpriority05.c
 *
 * DESCRIPTION
 *	setpriority05 - test for an expected failure by trying to change
 *			a process with an ID that is different from the
 *			test process
 *
 * ALGORITHM
 *	loop if that option was specified
 *	issue the system call with init's process id (1)
 *	check the errno value
 *	  issue a PASS message if we get EPERM - errno 1
 *	otherwise, the tests fails
 *	  issue a FAIL message
 *	  break any remaining tests
 *	  call cleanup
 *
 * USAGE:  <for command-line>
 *  setpriority05 [-c n] [-e] [-i n] [-I x] [-p x] [-t]
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
#include <pwd.h>

void cleanup(void);
void setup(void);

char *TCID = "setpriority05";
int TST_TOTAL = 1;
char nobody_uid[] = "nobody";
struct passwd *ltpuser;

int exp_enos[] = { EPERM, 0 };

int main(int ac, char **av)
{
	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */
	int new_val = 2;
	int init_val = 1;	/* the init process = id 1 */

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
		 * Try to access the init process.
		 * This should give an EPERM error.
		 */

		/* call the system call with the TEST() macro */
		TEST(setpriority(PRIO_PROCESS, init_val, new_val));

		if (TEST_RETURN == 0) {
			tst_resm(TFAIL, "call failed to produce expected error "
				 "- errno = %d - %s", TEST_ERRNO,
				 strerror(TEST_ERRNO));
		}

		TEST_ERROR_LOG(TEST_ERRNO);

		switch (TEST_ERRNO) {
		case EPERM:
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

	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	/* Set up the expected error numbers */
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