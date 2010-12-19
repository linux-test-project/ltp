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
 *	setreuid06.c
 *
 * DESCRIPTION
 *	Test that EPERM is set when setreuid is given an invalid user id.
 *
 * USAGE:  <for command-line>
 *	setreuid06 [-c n] [-e] [-i n] [-I x] [-P x] [-t]
 *	where,  -c n : Run n copies concurrently.
 *		-e   : Turn on errno logging.
 *		-i n : Execute test n times.
 *		-I x : Execute test for x seconds.
 *		-P x : Pause for x seconds between iterations.
 *		-t   : Turn on syscall timing.
 *
 * History
 *	07/2001 John George
 *		-Ported
 *
 * Restrictions
 *      Must be ran as non-root user - nobody recommended.
 */

#include <wait.h>
#include <limits.h>
#include <signal.h>
#include <errno.h>
#include <unistd.h>
#include <pwd.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "test.h"
#include "usctest.h"

#define INVAL_USER		 (USHRT_MAX-2)

char *TCID = "setreuid06";
int TST_TOTAL = 1;
int exp_enos[] = { EPERM, 0 };

char nobody_uid[] = "nobody";
struct passwd *ltpuser;

void setup(void);
void cleanup(void);

int main(int argc, char **argv)
{

	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */

	/* parse standard options */
	if ((msg = parse_opts(argc, argv, NULL, NULL)) !=
	    NULL) {
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);
	 }

	/*
	 * perform global setup for the test
	 */
	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		/* reset Tst_count in case we are looping */
		Tst_count = 0;

		TEST(setreuid(-1, INVAL_USER));
		if (TEST_RETURN != -1) {
			tst_resm(TFAIL, "%s did not fail as expected", TCID);
		} else if (TEST_ERRNO == EPERM) {
			TEST_ERROR_LOG(TEST_ERRNO);
			tst_resm(TPASS, "setreuid set errno to EPERM as "
				 "expected");
		} else {
			TEST_ERROR_LOG(TEST_ERRNO);
			tst_resm(TFAIL, "setreuid FAILED, expected 1 but "
				 "returned %d", TEST_ERRNO);
		}

	}
	cleanup();
	tst_exit();
	tst_exit();

}

/*
 * setup()
 *	performs all ONE TIME setup for this test
 */
void setup(void)
{

	tst_sig(FORK, DEF_HANDLER, cleanup);

	/* set the expected errnos... */
	TEST_EXP_ENOS(exp_enos);

	umask(0);

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

	TEST_PAUSE;
}

/*
 * cleanup()
 *	performs all the ONE TIME cleanup for this test at completion
 *	or premature exit
 */
void cleanup(void)
{
	/*
	 * print timing status if that option was specified
	 * print errno log if that option was specified
	 */
	TEST_CLEANUP;

}