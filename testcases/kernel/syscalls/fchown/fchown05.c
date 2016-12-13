/*
 * Copyright (c) International Business Machines  Corp., 2001
 *  07/2001 Ported by Wayne Boyer
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
 *  Verify that, fchown(2) succeeds to change the owner and group of a file
 *  specified by file descriptor to any numeric owner(uid)/group(gid) values
 *  when invoked by super-user.
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

#define FILE_MODE	S_IFREG | S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH
#define TESTFILE	"testfile"

TCID_DEFINE(fchown05);

static struct test_case_t {
	char *desc;
	uid_t user_id;
	gid_t group_id;
} tc[] = {
	{"Change Owner/Group ids", 700, 701},
	{"Change Owner id only", 702, -1},
	{"Change Owner id only", 703, 701},
	{"Change Group id only", -1, 704},
	{"Change Group id only", 703, 705},
	{NULL, 0, 0}
};

int TST_TOTAL = ARRAY_SIZE(tc);

static void setup(void);
static void cleanup(void);
static int fildes;

int main(int ac, char **av)
{
	struct stat stat_buf;
	int i, lc;
	uid_t user_id;
	gid_t group_id;

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		tst_count = 0;

		for (i = 0; tc[i].desc != NULL; i++) {
			user_id = tc[i].user_id;
			group_id = tc[i].group_id;

			TEST(FCHOWN(cleanup, fildes, user_id, group_id));

			if (TEST_RETURN == -1) {
				tst_resm(TFAIL | TTERRNO,
					 "fchown() Fails to %s", tc[i].desc);
				continue;
			}

			SAFE_FSTAT(cleanup, fildes, &stat_buf);

			if (user_id == (uid_t)-1)
				user_id = tc[i - 1].user_id;

			if (group_id == (gid_t)-1)
				group_id = tc[i - 1].group_id;

			if ((stat_buf.st_uid != user_id) ||
			    (stat_buf.st_gid != group_id)) {
				tst_resm(TFAIL, "%s: Incorrect owner"
					 "ship set, Expected %d %d",
					 TESTFILE, user_id, group_id);
			} else {
				tst_resm(TPASS,
					 "fchown() succeeds to %s of %s",
					 tc[i].desc, TESTFILE);
			}
		}
	}

	cleanup();
	tst_exit();
}

static void setup(void)
{
	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	tst_require_root();

	TEST_PAUSE;

	tst_tmpdir();

	fildes = SAFE_OPEN(cleanup, TESTFILE, O_RDWR | O_CREAT, FILE_MODE);
}

static void cleanup(void)
{
	if (fildes > 0 && close(fildes))
		tst_resm(TWARN | TERRNO, "close(%s) Failed", TESTFILE);

	tst_rmdir();
}
