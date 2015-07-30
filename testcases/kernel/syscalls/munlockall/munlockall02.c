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
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 */
/**************************************************************************
 *
 *    TEST IDENTIFIER	: munlockall02
 *
 *    EXECUTED BY	: root / superuser
 *
 *    TEST TITLE	: test for EPERM error value when run as non superuser
 *
 *    TEST CASE TOTAL	: 1
 *
 *    AUTHOR		: sowmya adiga<sowmya.adiga@wipro.com>
 *
 *    SIGNALS
 * 	Uses SIGUSR1 to pause before test if option set.
 * 	(See the parse_opts(3) man page).
 *
 *    DESCRIPTION
 *	Verify munlockall(2) returns -1 and sets errno to EPERM
 *	if the effective userid of the caller is not super-user.
 *	$
 * 	Setup:
 *	  Setup signal handling.
 *	  Pause for SIGUSR1 if option specified.
 *        Change effective user id to "nobody" user
 *     $
 * 	Test:
 *	 Loop if the proper options are given.
 *	  Execute system call
 *	  Check return code, if system call failed (return=-1) &&
 *	                    (errno set == expected errno)
 *	 Issue sys call pass with expected return value and errno.
 *	 otherwise,
 *	  Issue sys call fails with unexpected errno.
 *
 *
 * 	Cleanup:
 *      change effective user id to root
 * 	Print errno log and/or timing stats if options given
 *
 * USAGE:  <for command-line>
 *  munlockall02 [-c n] [-e] [-i n] [-I x] [-p x] [-t]
 *		where,		-c n : Run n copies concurrently
 *				-e   : Turn on errno logging.
 *				-h   : Show this help screen
 *				-i n : Execute test n times.
 *				-I x : Execute test for x seconds.
 *				-p   : Pause for SIGUSR1 before starting
 *                      	-P x : Pause for x seconds between iterations.
 *                       	 t   : Turn on syscall timing.
 *
 *
 *****************************************************************************/
#include <errno.h>
#include <pwd.h>
#include <sys/mman.h>
#include "test.h"

void setup();
void cleanup();

char *TCID = "munlockall02";
int TST_TOTAL = 1;

static char nobody_uid[] = "nobody";
struct passwd *ltpuser;

#if !defined(UCLINUX)

int main(int ac, char **av)
{
	int lc;

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	/* check looping state */
	for (lc = 0; TEST_LOOPING(lc); lc++) {

		tst_count = 0;

		TEST(munlockall());
		/* check return code */
		if ((TEST_RETURN == -1) && (TEST_ERRNO == EPERM)) {
			tst_resm(TPASS, "munlockall() failed"
				 " as expected for non-superuser" ":GOT EPERM");
		} else {
			tst_resm(TCONF, "munlockall() failed to produce "
				 "expected errno :%d Got : %d, %s. ***Some distros, such as Red Hat Enterprise Linux, support non-superuser munlockall calls.***",
				 EPERM, TEST_ERRNO, strerror(TEST_ERRNO));

		}
	}

	/* cleanup and exit */
	cleanup();
	tst_exit();

}

/* setup() - performs all ONE TIME setup for this test. */
void setup(void)
{
	tst_require_root();

	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	/* switch to nobody user */
	if ((ltpuser = getpwnam(nobody_uid)) == NULL) {
		tst_brkm(TBROK, NULL, "\"nobody\"user not present");
	}

	if (seteuid(ltpuser->pw_uid) == -1) {
		tst_brkm(TBROK, NULL, "seteuid failed to "
			 "to set the effective uid to %d", ltpuser->pw_uid);
	}

	TEST_PAUSE;
}

#else

int main(void)
{
	tst_resm(TINFO, "test is not available on uClinux");
	tst_exit();
}

#endif /* if !defined(UCLINUX) */

/*
 * cleanup() - performs all ONE TIME cleanup for this test at
 *		completion or premature exit.
 */
void cleanup(void)
{
	if (seteuid(0) == -1) {
		tst_resm(TWARN, "seteuid failed to "
			 "to set the effective uid to root");
		perror("setuid");
	}

}
