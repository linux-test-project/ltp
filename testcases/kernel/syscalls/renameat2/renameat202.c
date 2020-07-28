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
 *   Calls renameat2(2) with the flag RENAME_EXCHANGE and check that
 *   the content was swapped
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

char *TCID = "renameat202";

static int olddirfd;
static int newdirfd;
static int fd = -1;
static int cnt;

static const char content[] = "content";
const char *device = "/dev/vda";
static const char *fs_type = "ext4";


int TST_TOTAL = 1;

static void setup(void);
static void cleanup(void);
static void renameat2_verify(void);


int main(int ac, char **av)
{
	int lc;

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {

		tst_count = 0;

		TEST(renameat2(olddirfd, TEST_FILE,
				newdirfd, TEST_FILE2, RENAME_EXCHANGE));

		cnt++;

		renameat2_verify();
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

	olddirfd = SAFE_OPEN(cleanup, TEST_DIR, O_DIRECTORY);
	newdirfd = SAFE_OPEN(cleanup, TEST_DIR2, O_DIRECTORY);

	SAFE_FILE_PRINTF(cleanup, TEST_DIR TEST_FILE, "%s", content);

}

static void cleanup(void)
{
	if (olddirfd > 0 && close(olddirfd) < 0)
		tst_resm(TWARN | TERRNO, "close olddirfd failed");

	if (newdirfd > 0 && close(newdirfd) < 0)
		tst_resm(TWARN | TERRNO, "close newdirfd failed");

	if (fd > 0 && close(fd) < 0)
		tst_resm(TWARN | TERRNO, "close fd failed");

	remove("test_dir/test_file");
	remove("test_dir2/test_file2");
	SAFE_RMDIR(NULL, TEST_DIR);
	SAFE_RMDIR(NULL, TEST_DIR2);
	SAFE_UMOUNT(NULL, MNTPOINT);
	SAFE_RMDIR(NULL, MNTPOINT);
}
 void renameat2_verify(void)
{
	char str[BUFSIZ] = { 0 };
	struct stat st;
	char *emptyfile;
	char *contentfile;
	int readn, data_len;


	if (TEST_RETURN != 0) {
		tst_resm(TFAIL | TTERRNO, "renameat2() failed unexpectedly");
		return;
	}

	if (cnt % 2 == 1) {
		emptyfile = TEST_DIR TEST_FILE;
		contentfile = TEST_DIR2 TEST_FILE2;
	} else {
		emptyfile = TEST_DIR2 TEST_FILE2;
		contentfile = TEST_DIR TEST_FILE;
	}

	fd = SAFE_OPEN(cleanup, contentfile, O_RDONLY);

	SAFE_STAT(cleanup, emptyfile, &st);

	readn = SAFE_READ(cleanup, 0, fd, str, BUFSIZ);

	if (close(fd) < 0)
		tst_brkm(TERRNO | TFAIL, cleanup, "close fd failed");
	fd = 0;

	data_len = sizeof(content) - 1;
	if (readn != data_len) {
		tst_resm(TFAIL, "Wrong number of bytes read after renameat2(). "
				"Expect %d, got %d", data_len, readn);
		return;
	}
	if (strncmp(content, str, data_len)) {
		tst_resm(TFAIL, "File content changed after renameat2(). "
				"Expect '%s', got '%s'", content, str);
		return;
	}
	if (st.st_size) {
		tst_resm(TFAIL, "emptyfile has non-zero file size");
		return;
	}
	tst_resm(TPASS, "renameat2() test passed");
}
