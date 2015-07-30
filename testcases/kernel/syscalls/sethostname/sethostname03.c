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
 */

/****************************************************************************
 *    TEST IDENTIFIER	: sethostname03
 *
 *    EXECUTED BY	: root / superuser
 *
 *    TEST TITLE	: test for EPERM error value when sethostname(2) is
 *                        called from as non superuser
 *
 *    TEST CASE TOTAL	: 1
 *
 *    AUTHOR		: Suresh Babu V. <suresh.babu@wipro.com>
 *
 *    SIGNALS
 *      Uses SIGUSR1 to pause before test if option set.
 *      (See the parse_opts(3) man page).
 *
 *    DESCRIPTION
 *      Verify that, sethostname(2) returns -1 and sets errno to EPERM
 *      if the effective userid id of the caller is not super-user.
 *
 *  Setup:
 *   Setup signal handling.
 *   Save the current host name.
 *   Pause for SIGUSR1 if option specified.
 *
 *  Test:
 *   Loop if the proper options are given.
 *   Execute system call
 *   Check return code, if system call failed (return=-1)
 *	if errno set == expected errno
 *		Issue sys call fails with expected return value and errno.
 *	Otherwise,
 *		Issue sys call fails with unexpected errno.
 *
 *  Cleanup:
 *   Restore old hostname
 *   Print errno log and/or timing stats if options given
 *
 * Usage:  <for command-line>
 *  sethostname03 [-c n] [-e] [-i n] [-I x] [-P x] [-p] [-t]
 *	where,  -c n : Run n copies concurrently.
 *		-e   : Turn on errno logging.
 *		-i n : Execute test n times.
 *		-I x : Execute test for x seconds.
 *		-P x : Pause for x seconds between iterations.
 *		-p   : Pause for SIGUSR1 before starting
 *		-t   : Turn on syscall timing.
 *
 *************************************************************************/

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <pwd.h>
#include <linux/utsname.h>

#include "test.h"

#define MAX_LENGTH __NEW_UTS_LEN

char *TCID = "sethostname03";
int TST_TOTAL = 1;

static char ltpthost[] = "ltphost";
static char hname[MAX_LENGTH];

static char nobody_uid[] = "nobody";
struct passwd *ltpuser;

static void setup(void);
static void cleanup(void);

int main(int ac, char **av)
{
	int lc;

	tst_parse_opts(ac, av, NULL, NULL);

	/* Do initial setup */
	setup();

	/* Check for looping state if -i option is given */
	for (lc = 0; TEST_LOOPING(lc); lc++) {

		tst_count = 0;

		/* call sethostname() */
		TEST(sethostname(ltpthost, sizeof(ltpthost)));

		if ((TEST_RETURN == -1) && (TEST_ERRNO == EPERM)) {
			tst_resm(TPASS, "Expected Failure; Got EPERM");
		} else {
			tst_resm(TFAIL, "call failed to produce "
				 "expected error;  errno: %d : %s",
				 TEST_ERRNO, strerror(TEST_ERRNO));
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

	/* Switch to nobody user for correct error code collection */
	if ((ltpuser = getpwnam(nobody_uid)) == NULL) {
		tst_brkm(TBROK, NULL, "Required user \"nobody\" not"
			 " present");
	}

	if (seteuid(ltpuser->pw_uid) == -1) {
		tst_resm(TWARN, "seteuid failed to "
			 "to set the effective uid to %d", ltpuser->pw_uid);
		perror("seteuid");
	}

	/* Keep current hostname */
	if ((ret = gethostname(hname, sizeof(hname))) < 0) {
		tst_brkm(TBROK, NULL, "gethostname() failed while"
			 " getting current host name");
	}

	TEST_PAUSE;

}

/*
 * cleanup()  - performs all one time cleanup for this test
 *		completion or premature exit.
 */
void cleanup(void)
{
	int ret;

	/* Set effective user id back to root/super user */
	if (seteuid(0) == -1) {
		tst_resm(TWARN, "seteuid failed to "
			 "to set the effective uid to root");
		perror("seteuid");
	}

	/* Restore host name */
	if ((ret = sethostname(hname, strlen(hname))) < 0) {
		tst_resm(TWARN, "sethostname() failed while restoring"
			 " hostname to \"%s\"", hname);
	}

}
