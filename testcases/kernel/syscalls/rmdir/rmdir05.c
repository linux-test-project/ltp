/*
 * Copyright (c) 2000 Silicon Graphics, Inc.  All Rights Reserved.
 *    AUTHOR		: Bill Branum
 *    CO-PILOT		: Steve Shaw
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
 */
 /*
  * TEST CASES
  *  rmdir(2) test for errno(s) EINVAL, EMLINK, EFAULT
  */

#include <errno.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <string.h>
#include "test.h"
#include "safe_macros.h"

static void setup(void);
static void cleanup(void);

#if !defined(UCLINUX)
extern char *get_high_address();
int TST_TOTAL = 6;
#else
int TST_TOTAL = 4;
#endif

char *TCID = "rmdir05";

static struct stat stat_buf;
static char dir_name[256];

static char *bad_addr = NULL;

int main(int argc, char **argv)
{
	int lc;

	tst_parse_opts(argc, argv, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		tst_count = 0;

		/*
		 * TEST CASE: 1
		 * path points to the current directory
		 */
		TEST(rmdir("."));

		if (TEST_RETURN == -1) {
			if (TEST_ERRNO & (EBUSY | ENOTEMPTY)) {
				/* For functionality tests, verify that the
				 * directory wasn't removed.
				 */
				if (stat(".", &stat_buf) == -1) {
					tst_resm(TFAIL,
						 "rmdir(\".\") removed the current working directory when it should have failed.");
				} else {
					tst_resm(TPASS,
						 "rmdir(\".\") failed to remove the current working directory. Returned %d : %s",
						 TEST_ERRNO,
						 strerror(TEST_ERRNO));
				}
			} else {
				tst_resm(TFAIL,
					 "rmdir(\".\") failed with errno %d : %s but expected %d (EBUSY)",
					 TEST_ERRNO,
					 strerror(TEST_ERRNO), EBUSY);
			}
		} else {
			tst_resm(TFAIL,
				 "rmdir(\".\") succeeded unexpectedly.");
		}

		/*
		 * TEST CASE: 2
		 * path points to the "." (dot) entry of a directory
		 */
		tst_resm(TCONF, "rmdir on \"dir/.\" supported on Linux");

		tst_resm(TCONF,
			 "linked directories test not implemented on Linux");

		/*
		 * TEST CASE: 4
		 * path argument points below the minimum allocated address space
		 */
#if !defined(UCLINUX)
		TEST(rmdir(bad_addr));

		if (TEST_RETURN == -1) {
		}

		if (TEST_RETURN == -1) {
			if (TEST_ERRNO == EFAULT) {
				tst_resm(TPASS,
					 "rmdir() - path argument points below the minimum allocated address space failed as expected with errno %d : %s",
					 TEST_ERRNO,
					 strerror(TEST_ERRNO));
			} else {
				tst_resm(TFAIL,
					 "rmdir() - path argument points below the minimum allocated address space failed with errno %d : %s but expected %d (EFAULT)",
					 TEST_ERRNO,
					 strerror(TEST_ERRNO), EFAULT);
			}
		} else {
			tst_resm(TFAIL,
				 "rmdir() - path argument points below the minimum allocated address space succeeded unexpectedly.");
		}

		/*
		 * TEST CASE: 5
		 * path argument points above the maximum allocated address space
		 */

		TEST(rmdir(get_high_address()));

		if (TEST_RETURN == -1) {
		}

		if (TEST_RETURN == -1) {
			if (TEST_ERRNO == EFAULT) {
				tst_resm(TPASS,
					 "rmdir() - path argument points above the maximum allocated address space failed as expected with errno %d : %s",
					 TEST_ERRNO,
					 strerror(TEST_ERRNO));
			} else {
				tst_resm(TFAIL,
					 "rmdir() - path argument points above the maximum allocated address space failed with errno %d : %s but expected %d (EFAULT)",
					 TEST_ERRNO,
					 strerror(TEST_ERRNO), EFAULT);
			}
		} else {
			tst_resm(TFAIL,
				 "rmdir() - path argument points above the maximum allocated address space succeeded unexpectedly.");
		}
#endif

		/*
		 * TEST CASE: 6
		 * able to remove a directory
		 */

		if (mkdir(dir_name, 0777) != 0) {
			tst_brkm(TBROK, cleanup,
				 "mkdir(\"%s\") failed with errno %d : %s",
				 dir_name, errno, strerror(errno));
		}

		TEST(rmdir(dir_name));

		if (TEST_RETURN == -1) {
			tst_resm(TFAIL,
				 "rmdir(\"%s\") failed when it should have passed. Returned %d : %s",
				 dir_name, TEST_ERRNO, strerror(TEST_ERRNO));
		} else {
			/* Verify the directory was removed. */
			if (stat(dir_name, &stat_buf) != 0) {
				tst_resm(TPASS,
					 "rmdir(\"%s\") removed the directory as expected.",
					 dir_name);
			} else {
				tst_resm(TFAIL,
					 "rmdir(\"%s\") returned a zero exit status but failed to remove the directory.",
					 dir_name);
			}
		}

	}

	cleanup();
	tst_exit();
}

void setup(void)
{
	tst_sig(FORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;

	tst_tmpdir();

	/* Create a directory. */
	SAFE_MKDIR(cleanup, "dir1", 0777);

	/* Create a unique directory name. */
	sprintf(dir_name, "./dir_%d", getpid());

#if !defined(UCLINUX)
	bad_addr = mmap(0, 1, PROT_NONE,
			MAP_PRIVATE_EXCEPT_UCLINUX | MAP_ANONYMOUS, 0, 0);
	if (bad_addr == MAP_FAILED) {
		tst_brkm(TBROK, cleanup, "mmap failed");
	}
#endif
}

void cleanup(void)
{
	tst_rmdir();
}
