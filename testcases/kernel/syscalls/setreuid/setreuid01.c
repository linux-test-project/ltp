/*
 * Copyright (c) 2000 Silicon Graphics, Inc.  All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it would be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * Further, this software is distributed without any warranty that it is
 * free of the rightful claim of any third person regarding infringement
 * or the like.  Any license provided herein, whether implied or
 * otherwise, applies only to this software file.  Patent licenses, if
 * any, provided herein do not apply to combinations of this program with
 * other software, or any other product whatsoever.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Contact information: Silicon Graphics, Inc., 1600 Amphitheatre Pkwy,
 * Mountain View, CA  94043, or:
 *
 * http://www.sgi.com
 *
 * For further information regarding this notice, see:
 *
 * http://oss.sgi.com/projects/GenInfo/NoticeExplan/
 *
 * Author: William Roske
 * Co-pilot: Dave Fenner
 */

/*
 * Testcase to test the basic functionality of setreuid(2) system call.
 */

#include <errno.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>

#include "test.h"
#include "compat_16.h"

static void setup(void);
static void cleanup(void);

TCID_DEFINE(setreuid01);
int TST_TOTAL = 5;

static uid_t ruid, euid;	/* real and effective user ids */

int main(int ac, char **av)
{
	int lc;

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {

		tst_count = 0;

		/*
		 * TEST CASE:
		 *  Don't change either real or effective uid
		 */
		ruid = getuid();	/* get real uid */
		UID16_CHECK(ruid, setreuid, cleanup);

		euid = geteuid();	/* get effective uid */
		UID16_CHECK(euid, setreuid, cleanup);

		TEST(SETREUID(cleanup, -1, -1));

		if (TEST_RETURN == -1) {
			tst_resm(TFAIL,
				 "setreuid -  Don't change either real or effective uid failed, errno=%d : %s",
				 TEST_ERRNO, strerror(TEST_ERRNO));
		} else {
			tst_resm(TPASS,
				 "setreuid -  Don't change either real or effective uid returned %ld",
				 TEST_RETURN);
		}

		/*
		 * TEST CASE:
		 *  change effective to effective uid
		 */

		TEST(SETREUID(cleanup, -1, euid));

		if (TEST_RETURN == -1) {
			tst_resm(TFAIL,
				 "setreuid -  change effective to effective uid failed, errno=%d : %s",
				 TEST_ERRNO, strerror(TEST_ERRNO));
		} else {
			tst_resm(TPASS,
				 "setreuid -  change effective to effective uid returned %ld",
				 TEST_RETURN);
		}

		/*
		 * TEST CASE:
		 *  change real to real uid
		 */

		TEST(SETREUID(cleanup, ruid, -1));

		if (TEST_RETURN == -1) {
			tst_resm(TFAIL,
				 "setreuid -  change real to real uid failed, errno=%d : %s",
				 TEST_ERRNO, strerror(TEST_ERRNO));
		} else {
			tst_resm(TPASS,
				 "setreuid -  change real to real uid returned %ld",
				 TEST_RETURN);
		}

		/*
		 * TEST CASE:
		 *  change effective to real uid
		 */

		TEST(SETREUID(cleanup, -1, ruid));

		if (TEST_RETURN == -1) {
			tst_resm(TFAIL,
				 "setreuid -  change effective to real uid failed, errno=%d : %s",
				 TEST_ERRNO, strerror(TEST_ERRNO));
		} else {
			tst_resm(TPASS,
				 "setreuid -  change effective to real uid returned %ld",
				 TEST_RETURN);
		}

		/*
		 * TEST CASE:
		 *  try to change real to current real
		 */

		TEST(SETREUID(cleanup, ruid, ruid));

		if (TEST_RETURN == -1) {
			tst_resm(TFAIL,
				 "setreuid -  try to change real to current real failed, errno=%d : %s",
				 TEST_ERRNO, strerror(TEST_ERRNO));
		} else {
			tst_resm(TPASS,
				 "setreuid -  try to change real to current real returned %ld",
				 TEST_RETURN);
		}

	}

	cleanup();
	tst_exit();
}

static void setup(void)
{
	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;

	tst_tmpdir();
}

static void cleanup(void)
{
	tst_rmdir();
}
