/*
 * Copyright (c) International Business Machines  Corp., 2001
 *
 * This program is free software;  you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY;  without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
 * the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program;  if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

/*
 * Test Description:
 *  Verify that, lchown(2) succeeds to change the owner and group of a file
 *  specified by path to any numeric owner(uid)/group(gid) values when invoked
 *  by super-user.
 *
 * Expected Result:
 *  lchown(2) should return 0 and the ownership set on the file should match
 *  the numeric values contained in owner and group respectively.
 *
 * HISTORY
 *	07/2001 Ported by Wayne Boyer
 *	11/2010 Code cleanup by Cyril Hrubis chrubis@suse.cz
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <signal.h>

#include "test.h"
#include "safe_macros.h"
#include "compat_16.h"

#define FILE_MODE	(S_IFREG|S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH)
#define TESTFILE	"testfile"
#define SFILE		"slink_file"

TCID_DEFINE(lchown01);
int TST_TOTAL = 5;

struct test_case_t {
	char *desc;
	uid_t user_id;
	gid_t group_id;
};

static struct test_case_t test_cases[] = {
	{"Change Owner/Group ids", 700, 701},
	{"Change Owner id only", 702, -1},
	{"Change Owner/Group ids", 703, 701},
	{"Change Group id only", -1, 704},
	{"Change Group/Group ids", 703, 705},
	{"Change none", -1, -1},
	{NULL, 0, 0}
};

static void setup(void);
static void cleanup(void);

int main(int argc, char *argv[])
{
	struct stat stat_buf;
	int lc;
	int i;

	tst_parse_opts(argc, argv, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {

		tst_count = 0;

		for (i = 0; test_cases[i].desc != NULL; i++) {
			uid_t user_id = test_cases[i].user_id;
			gid_t group_id = test_cases[i].group_id;
			char *test_desc = test_cases[i].desc;

			/*
			 * Call lchown(2) with different user id and
			 * group id (numeric values) to set it on
			 * symlink of testfile.
			 */
			TEST(LCHOWN(cleanup, SFILE, user_id, group_id));

			if (TEST_RETURN == -1) {
				tst_resm(TFAIL,
					 "lchown() Fails to %s, errno %d",
					 test_desc, TEST_ERRNO);
				continue;
			}

			if (lstat(SFILE, &stat_buf) < 0) {
				tst_brkm(TFAIL, cleanup, "lstat(2) "
					 "%s failed, errno %d",
					 SFILE, TEST_ERRNO);
			}

			if (user_id == -1) {
				if (i > 0)
					user_id =
					    test_cases[i - 1].user_id;
				else
					user_id = geteuid();
			}

			if (group_id == -1) {
				if (i > 0)
					group_id =
					    test_cases[i - 1].group_id;
				else
					group_id = getegid();
			}

			/*
			 * Check for expected Ownership ids
			 * set on testfile.
			 */
			if ((stat_buf.st_uid != user_id) ||
			    (stat_buf.st_gid != group_id)) {
				tst_resm(TFAIL,
					 "%s: incorrect ownership set, "
					 "Expected %d %d", SFILE,
					 user_id, group_id);
			} else {
				tst_resm(TPASS, "lchown() succeeds to "
					 "%s of %s", test_desc, SFILE);
			}
		}
	}

	cleanup();
	tst_exit();
}

static void setup(void)
{
	int fd;

	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	tst_require_root();

	TEST_PAUSE;
	tst_tmpdir();

	if ((fd = open(TESTFILE, O_RDWR | O_CREAT, FILE_MODE)) == -1) {
		tst_brkm(TBROK, cleanup, "open failed");
	}
	SAFE_CLOSE(cleanup, fd);

	SAFE_SYMLINK(cleanup, TESTFILE, SFILE);
}

static void cleanup(void)
{
	tst_rmdir();
}
