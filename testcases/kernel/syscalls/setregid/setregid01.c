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
 * Testcase to test the basic functionality of setregid(2) systemm call.
 */

#include <errno.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>

#include "test.h"
#include "usctest.h"
#include "compat_16.h"

static void setup(void);
static void cleanup(void);

TCID_DEFINE(setregid01);
int TST_TOTAL = 5;

static gid_t gid, egid;	/* current real and effective group id */

int main(int ac, char **av)
{
	int lc;
	char *msg;

	if ((msg = parse_opts(ac, av, NULL, NULL)) != NULL)
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {

		tst_count = 0;

		/*
		 * TEST CASE:
		 *  Dont change either real or effective gid
		 */
		gid = getgid();
		GID16_CHECK(gid, setregid, cleanup);

		egid = getegid();
		GID16_CHECK(egid, setregid, cleanup);

		TEST(SETREGID(cleanup, -1, -1));

		if (TEST_RETURN == -1) {
			TEST_ERROR_LOG(TEST_ERRNO);
			tst_resm(TFAIL,
				 "setregid -  Dont change either real or effective gid failed, errno=%d : %s",
				 TEST_ERRNO, strerror(TEST_ERRNO));
		} else {
			if (STD_FUNCTIONAL_TEST) {
				tst_resm(TPASS,
					 "setregid -  Dont change either real or effective gid returned %ld",
					 TEST_RETURN);
			}
		}

		/*
		 * TEST CASE:
		 *  change effective to effective gid
		 */

		TEST(SETREGID(cleanup, -1, egid));

		if (TEST_RETURN == -1) {
			TEST_ERROR_LOG(TEST_ERRNO);
			tst_resm(TFAIL,
				 "setregid -  change effective to effective gid failed, errno=%d : %s",
				 TEST_ERRNO, strerror(TEST_ERRNO));
		} else {
			if (STD_FUNCTIONAL_TEST) {
				tst_resm(TPASS,
					 "setregid -  change effective to effective gid returned %ld",
					 TEST_RETURN);
			}
		}

		/*
		 * TEST CASE:
		 *  change real to real gid
		 */

		TEST(SETREGID(cleanup, gid, -1));

		if (TEST_RETURN == -1) {
			TEST_ERROR_LOG(TEST_ERRNO);
			tst_resm(TFAIL,
				 "setregid -  change real to real gid failed, errno=%d : %s",
				 TEST_ERRNO, strerror(TEST_ERRNO));
		} else {
			if (STD_FUNCTIONAL_TEST) {
				tst_resm(TPASS,
					 "setregid -  change real to real gid returned %ld",
					 TEST_RETURN);
			}
		}

		/*
		 * TEST CASE:
		 *  change effective to real gid
		 */

		TEST(SETREGID(cleanup, -1, gid));

		if (TEST_RETURN == -1) {
			TEST_ERROR_LOG(TEST_ERRNO);
			tst_resm(TFAIL,
				 "setregid -  change effective to real gid failed, errno=%d : %s",
				 TEST_ERRNO, strerror(TEST_ERRNO));
		} else {
			if (STD_FUNCTIONAL_TEST) {
				tst_resm(TPASS,
					 "setregid -  change effective to real gid returned %ld",
					 TEST_RETURN);
			}
		}

		/*
		 * TEST CASE:
		 *  try to change real to current real
		 */

		TEST(SETREGID(cleanup, gid, gid));

		if (TEST_RETURN == -1) {
			TEST_ERROR_LOG(TEST_ERRNO);
			tst_resm(TFAIL | TTERRNO, "setregid failed");
		} else {
			if (STD_FUNCTIONAL_TEST) {
				tst_resm(TPASS, "setregid return %ld",
					 TEST_RETURN);
			}
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
	TEST_CLEANUP;

	tst_rmdir();
}
