/*
 * Copyright (c) Wipro Technologies Ltd, 2002.  All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it would be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program;  if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 *
 */
/**********************************************************
 *
 *    TEST IDENTIFIER   : setdomainname03
 *
 *    EXECUTED BY       : root / superuser
 *
 *    TEST TITLE        : test for EPERM error value when run as non superuser
 *
 *    TEST CASE TOTAL   : 1
 *
 *    AUTHOR            : Saji Kumar.V.R <saji.kumar@wipro.com>
 *
 *    SIGNALS
 *      Uses SIGUSR1 to pause before test if option set.
 *      (See the parse_opts(3) man page).
 *
 *    DESCRIPTION
 * 	Verify that, setdomainname(2) returns -1 and sets errno to EPERM
 * 	if the effective user id of the caller is not super-user.
 *
 * Algorithm:
 *  Setup:
 *   Setup signal handling.
 *   Pause for SIGUSR1 if option specified.
 *   save current domainname
 *   change effective user id to "nobody" user
 *
 *  Test:
 *   Loop if the proper options are given.
 *   Execute system call
 *   Check return code, Check return code, if (system call failed (return=-1)) &
 *			(errno set == expected errno)
 *   		Issue sys call fails with expected return value and errno.
 *   	Otherwise,
 *		Issue sys call fails with unexpected errno.
 *   Otherwise,
 *	Issue sys call returns unexpected value.
 *
 *  Cleanup:
 *   Change effective user id to root
 *   Restore old domainname
 *   Print errno log and/or timing stats if options given
 *
 * Usage:  <for command-line>
 *  setdomainname03 [-c n] [-e] [-i n] [-I x] [-P x] [-t] [-h] [-f] [-p]
 *	where,  -c n : Run n copies concurrently.
 *		-e   : Turn on errno logging.
 *		-h   : Show help screen
 *		-f   : Turn off functional testing
 *		-i n : Execute test n times.
 *		-I x : Execute test for x seconds.
 *		-p   : Pause for SIGUSR1 before starting
 *		-P x : Pause for x seconds between iterations.
 *		-t   : Turn on syscall timing.
 *
 ****************************************************************/

#include <string.h>
#include <errno.h>
#include <pwd.h>
#include <linux/utsname.h>

#include "test.h"

#define MAX_NAME_LEN __NEW_UTS_LEN

char *TCID = "setdomainname03";
int TST_TOTAL = 1;

static char nobody_uid[] = "nobody";
struct passwd *ltpuser;

static char test_domain_name[MAX_NAME_LEN] = "test_dom";
static char old_domain_name[MAX_NAME_LEN];

static void setup();		/* setup function for the tests */
static void cleanup();		/* cleanup function for the tests */

int main(int ac, char **av)
{
	int lc;

	tst_parse_opts(ac, av, NULL, NULL);

	/*
	 * Invoke setup function to call individual test setup functions
	 * for the test which run as root/super-user.
	 */
	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {

		tst_count = 0;

		/*
		 * Call setdomainname(2)
		 */
		TEST(setdomainname(test_domain_name, MAX_NAME_LEN));
		if ((TEST_RETURN == -1) && (TEST_ERRNO == EPERM)) {
			tst_resm(TPASS, "expected failure; Got EPERM");
		} else {
			tst_resm(TFAIL, "Call failed to produce "
				 "expected error;  Expected errno: %d "
				 "Got : %d, %s", EPERM, TEST_ERRNO,
				 strerror(TEST_ERRNO));
		}

	}

	/*
	 * Invoke cleanup() to delete the test directories created
	 * in the setup().
	 */
	cleanup();
	tst_exit();

}

/*
 * setup(void) - performs all ONE TIME setup for this test.
 */
void setup(void)
{
	tst_require_root();

	/* Capture unexpected signals */
	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	/* Switch to nobody user for correct error code collection */
	if ((ltpuser = getpwnam(nobody_uid)) == NULL) {
		tst_brkm(TBROK, NULL, "\"nobody\" user not present");
	}
	if (seteuid(ltpuser->pw_uid) == -1) {
		tst_resm(TWARN, "seteuid failed to "
			 "to set the effective uid to %d", ltpuser->pw_uid);
		perror("seteuid");
	}

	/* Save current domainname */
	if ((getdomainname(old_domain_name, MAX_NAME_LEN)) < 0) {
		tst_brkm(TBROK, NULL, "getdomainname() failed while"
			 " getting current domain name");
	}

	TEST_PAUSE;

}

/*
 * cleanup() - Performs all ONE TIME cleanup for this test at
 */
void cleanup(void)
{

	/* Set effective user id back to root */
	if (seteuid(0) == -1) {
		tst_resm(TWARN, "seteuid failed to "
			 "to set the effective uid to root");
		perror("seteuid");
	}

	/* Restore domain name */
	if ((setdomainname(old_domain_name, strlen(old_domain_name)))
	    < 0) {
		tst_resm(TWARN, "setdomainname() failed while restoring"
			 " domainname to \"%s\"", old_domain_name);
	}

}
