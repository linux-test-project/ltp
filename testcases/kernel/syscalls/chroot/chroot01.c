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
 * 	chroot01.c
 *
 * DESCRIPTION
 *	Testcase to check the whether chroot sets errno to EPERM.
 *
 * ALGORITHM
 *	As a non-root user attempt to perform chroot() to a directory. The
 *	chroot() call should fail with EPERM
 *
 * USAGE:  <for command-line>
 *  chroot01 [-c n] [-e] [-i n] [-I x] [-P x] [-t]
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
 * 	Must be run as non-root user.
 */

#include <stdio.h>
#include <errno.h>
#include "test.h"
#include "usctest.h"
#include <pwd.h>

char *TCID = "chroot01";
int TST_TOTAL = 1;
int fail;

char path[] = "/tmp";
int exp_enos[] = { EPERM, 0 };
char nobody_uid[] = "nobody";
struct passwd *ltpuser;

void setup(void);
void cleanup(void);

int
main(int ac, char **av)
{
	int lc;
	char *msg;

	if ((msg = parse_opts(ac, av, NULL, NULL)) != NULL)
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);

	setup();

	TEST_EXP_ENOS(exp_enos);

	for (lc = 0; TEST_LOOPING(lc); lc++) {

		Tst_count = 0;

		TEST(chroot(path));

		if (TEST_RETURN != -1)
			tst_resm(TFAIL, "call succeeded unexpectedly");
		else if (errno != EPERM)
			tst_resm(TFAIL|TTERRNO, "chroot failed unexpectedly");
		else
			tst_resm(TPASS, "chroot set errno to EPERM.");
	}
	cleanup();

	tst_exit();

}

void
setup(void)
{
	tst_require_root(NULL);

	tst_tmpdir();

	if ((ltpuser = getpwnam(nobody_uid)) == NULL)
		tst_brkm(TBROK|TERRNO, cleanup, "getpwnam(\"nobody\") failed");

	if (seteuid(ltpuser->pw_uid) == -1)
		tst_brkm(TBROK|TERRNO, cleanup, "seteuid to nobody failed");

	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;
}

void
cleanup(void)
{
	TEST_CLEANUP;

	if (seteuid(0) == -1)
		tst_brkm(TBROK|TERRNO, NULL, "setuid(0) failed");

	tst_rmdir();
}