/*
 *
 *   Copyright (c) Wipro Technologies, 2002.  All Rights Reserved.
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
 *
 */

/*********************************************************************
 *    TEST IDENTIFIER	: sethostname01
 *
 *    EXECUTED BY	: root / superuser
 *
 *    TEST TITLE	: Basic test for sethostname(2)
 *
 *    TEST CASE TOTAL	: 1
 *
 *    AUTHOR		: Suresh Babu V. <suresh.babu@wipro.com>
 *
 *    SIGNALS
 *	Uses SIGUSR1 to pause before test if option set.
 *	(See the parse_opts(3) man page).
 *
 *    DESCRIPTION$
 *      This is a Phase I test for the sethostname(2) system call.
 *      It is intended to provide a limited exposure of the system call.
 *
 *      Setup:
 *        Setup signal handling.
 *        Save the current hostname.
 *        Pause for SIGUSR1 if option specified.
 *
 *      Test:
 *       Loop if the proper options are given.
 *        Execute system call
 *        Check return code, if system call failed (return=-1)
 *              Log the errno and Issue a FAIL message.
 *        Otherwise, Issue a PASS message.
 *      Cleanup:
 *        Restore old host name.
 *        Print errno log and/or timing stats if options given
 *
 * Usage:  <for command-line>
 *  sethostname01 [-c n] [-i n] [-I x] [-P x] [-p] [-t] [-h]
 *	where,  -c n : Run n copies concurrently.
 *		-i n : Execute test n times.
 *		-I x : Execute test for x seconds.
 *		-p   : Pause for SIGUSR1 before starting
 *		-P x : Pause for x seconds between iterations.
 *		-t   : Turn on syscall timing.
 *		-h   : Display usage information
 *
 *
 ******************************************************************/

#include <string.h>
#include <errno.h>
#include <linux/utsname.h>

#include "test.h"

#define MAX_LENGTH __NEW_UTS_LEN

static void setup();
static void cleanup();

char *TCID = "sethostname01";
int TST_TOTAL = 1;
static char hname[MAX_LENGTH];	/* host name */

int main(int ac, char **av)
{
	int lc;

	char ltphost[] = "ltphost";	/* temporary host name to set */

	tst_parse_opts(ac, av, NULL, NULL);

	/* Do initial setup. */
	setup();

	/* check -c option for looping. */
	for (lc = 0; TEST_LOOPING(lc); lc++) {

		tst_count = 0;

		/* Call sethostname(2) */
		TEST(sethostname(ltphost, sizeof(ltphost)));

		/* check return code */
		if (TEST_RETURN == -1) {
			tst_resm(TFAIL, "sethostname() failed , errno=%d : %s",
				 TEST_ERRNO, strerror(TEST_ERRNO));
		} else {
			tst_resm(TPASS, "sethostname() returned %ld,"
				 " Hostname set to \"%s\"", TEST_RETURN,
				 ltphost);
		}

	}

	/* cleanup and exit */
	cleanup();
	tst_exit();

}

/*
 * setup() - performs all one time setup for this test.
 */
void setup(void)
{
	int ret;

	tst_require_root();

	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	/* Store the existing hostname to retain it before exiting */
	if ((ret = gethostname(hname, sizeof(hname))) < 0) {
		tst_brkm(TBROK, NULL, "gethostname() failed while getting"
			 " current host name");
	}

	TEST_PAUSE;

}

/*
 * cleanup() -	performs all one time cleanup for this test
 *		completion or premature exit.
 */
void cleanup(void)
{
	int ret;

	/* Set the host name back to original name */
	if ((ret = sethostname(hname, strlen(hname))) < 0) {
		tst_resm(TWARN, "sethostname() failed while restoring"
			 " hostname to \"%s\": %s", hname, strerror(errno));
	}

}
