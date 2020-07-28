/*
 * Copyright (c) 2015 Cedric Hnyda <chnyda@suse.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it would be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write the Free Software Foundation,
 * Inc.,  51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

 /* Description:
 *   Verify that:
 *   1) renameat2(2) returns -1 and sets errno to EEXIST because newpath
 *		already exists and the flag RENAME_NOREPLACE is used.
 *   2) renameat2(2) returns 0.
 *   3) renameat2(2) returns -1 and sets errno to ENOENT because the flag
 *		RENAME_EXCHANGE is used and newpath does not exist.
 *   4) renameat2(2) returns 0 because the flag RENAME_NOREPLACE is used,
 *		both olddirfd and newdirfd are valid and oldpath exists and
 *		newpath does not exist.
 *   5) renameat2(2) returns -1 and sets errno to EINVAL because
 *		RENAME_NOREPLACE and RENAME_EXCHANGE are used together
 *   6) renameat2(2) returns -1 and sets errno to EINVAL because
 *		RENAME_WHITEOUT and RENAME_EXCHANGE are used together
 */

#define _GNU_SOURCE

#include "test.h"
#include "safe_macros.h"
#include "lapi/fcntl.h"
#include "renameat2.h"

#define MNTPOINT  "mntpoint"
#define DIR_MODE  (S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH)
#define TEST_DIR "mntpoint/test_dir/"
#define TEST_DIR2 "mntpoint/test_dir2/"

#define TEST_FILE "test_file"
#define TEST_FILE2 "test_file2"
#define TEST_FILE3 "test_file3"
#define NON_EXIST "non_exist"

char *TCID = "renameat201";

static int olddirfd;
static int newdirfd;
const char *device = "/dev/vda";
static const char *fs_type = "ext4";

static struct test_case {
	int *olddirfd;
	const char *oldpath;
	int *newdirfd;
	const char *newpath;
	int flags;
	int exp_errno;
} test_cases[] = {
	{&olddirfd, TEST_FILE, &newdirfd, TEST_FILE2, RENAME_NOREPLACE, EEXIST},
	{&olddirfd, TEST_FILE, &newdirfd, TEST_FILE2, RENAME_EXCHANGE, 0},
	{&olddirfd, TEST_FILE, &newdirfd, NON_EXIST, RENAME_EXCHANGE, ENOENT},
	{&olddirfd, TEST_FILE, &newdirfd, TEST_FILE3, RENAME_NOREPLACE, 0},
	{&olddirfd, TEST_FILE, &newdirfd, TEST_FILE2, RENAME_NOREPLACE
				| RENAME_EXCHANGE, EINVAL},
	{&olddirfd, TEST_FILE, &newdirfd, TEST_FILE2, RENAME_WHITEOUT
				| RENAME_EXCHANGE, EINVAL}
};

int TST_TOTAL = ARRAY_SIZE(test_cases);

static void setup(void);
static void cleanup(void);
static void renameat2_verify(const struct test_case *test);


int main(int ac, char **av)
{
	int i;
	int lc;

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	for (lc = 0; lc < TEST_LOOPING(lc); lc++) {
		tst_count = 0;

		for (i = 0; i < TST_TOTAL; i++)
			renameat2_verify(&test_cases[i]);
	}

	cleanup();
	tst_exit();
}

static void setup(void)
{
	if ((tst_kvercmp(3, 15, 0)) < 0) {
		tst_brkm(TCONF, NULL,
			"This test can only run on kernels that are 3.15. and higher");
	}



	rmdir(MNTPOINT);
	SAFE_MKDIR(cleanup, MNTPOINT, DIR_MODE);
	SAFE_MOUNT(cleanup, device, MNTPOINT, fs_type, 0, NULL);

	SAFE_MKDIR(cleanup, TEST_DIR, 0700);
	SAFE_MKDIR(cleanup, TEST_DIR2, 0700);

	SAFE_TOUCH(cleanup, TEST_DIR TEST_FILE, 0600, NULL);
	SAFE_TOUCH(cleanup, TEST_DIR2 TEST_FILE2, 0600, NULL);
	SAFE_TOUCH(cleanup, TEST_DIR TEST_FILE3, 0600, NULL);

	olddirfd = SAFE_OPEN(cleanup, TEST_DIR, O_DIRECTORY);
	newdirfd = SAFE_OPEN(cleanup, TEST_DIR2, O_DIRECTORY);
}

static void cleanup(void)
{
	if (olddirfd > 0 && close(olddirfd) < 0)
		tst_resm(TWARN | TERRNO, "close olddirfd failed");

	if (newdirfd > 0 && close(newdirfd) < 0)
		tst_resm(TWARN | TERRNO, "close newdirfd failed");
	remove("test_dir/test_file1");
	remove("test_dir/test_file3");
	remove("test_dir2/test_file2");
	remove("test_dir2/test_file3");

	SAFE_RMDIR(NULL, TEST_DIR);
	SAFE_RMDIR(NULL, TEST_DIR2);
	SAFE_UMOUNT(NULL, MNTPOINT);
	SAFE_RMDIR(NULL, MNTPOINT);


}

static void renameat2_verify(const struct test_case *test)
{
	TEST(renameat2(*(test->olddirfd), test->oldpath,
			*(test->newdirfd), test->newpath, test->flags));


	if (test->exp_errno && TEST_RETURN != -1) {
		tst_resm(TFAIL, "renameat2() succeeded unexpectedly");
		return;
	}

	if (test->exp_errno == 0 && TEST_RETURN != 0) {
		tst_resm(TFAIL | TTERRNO, "renameat2() failed unexpectedly");
		return;
	}

	if (test->exp_errno == TEST_ERRNO) {
		tst_resm(TPASS | TTERRNO,
		"renameat2() returned the expected value");
		return;
	}

	tst_resm(TFAIL | TTERRNO,
		"renameat2() got unexpected return value: expected: %d - %s",
			test->exp_errno, tst_strerrno(test->exp_errno));

}
