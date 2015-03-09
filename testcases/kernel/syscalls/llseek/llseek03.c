/*
 * Copyright (c) 2014 Fujitsu Ltd.
 * Author: Xiaoguang Wang <wangxg.fnst@cn.fujitsu.com>
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
 */

/*
 * Description:
 * Verify that,
 *    1. llseek() succeeds to set the file pointer position to the current
 *  specified location, when 'whence' value is set to SEEK_CUR and the data
 *  read from the specified location should match the expected data.
 *    2. llseek() succeeds to set the file pointer position to the end of
 *  the file when 'whence' value set to SEEK_END and any attempts to read
 *  from that position should return 0.
 *
 */

#define _GNU_SOURCE

#include <unistd.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <utime.h>
#include <string.h>
#include <signal.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <inttypes.h>

#include "test.h"
#include "safe_macros.h"

#define TEST_FILE "testfile"

static void setup(void);
static void cleanup(void);

static void testfunc_seekcur(void);
static void testfunc_seekend(void);

static void (*testfunc[])(void) = { testfunc_seekcur, testfunc_seekend };

char *TCID = "llseek03";
int TST_TOTAL = 2;

static size_t file_size;

int main(int ac, char **av)
{
	int i, lc;

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		tst_count = 0;

		for (i = 0; i < TST_TOTAL; i++)
			(*testfunc[i])();
	}

	cleanup();
	tst_exit();
}

static void setup(void)
{
	int fd;
	struct stat stat_buf;

	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	tst_tmpdir();

	TEST_PAUSE;

	fd = SAFE_CREAT(cleanup, TEST_FILE, 0644);

	#define STR "abcdefgh"
	SAFE_WRITE(cleanup, 1, fd, STR, sizeof(STR) - 1);

	SAFE_FSTAT(cleanup, fd, &stat_buf);

	SAFE_CLOSE(cleanup, fd);

	file_size = stat_buf.st_size;
}

static void testfunc_seekcur(void)
{
	int fd;
	static char read_buf[BUFSIZ];

	/* reopen TEST_FILE and file offset will be 0 */
	fd = SAFE_OPEN(cleanup, TEST_FILE, O_RDONLY);

	/* after read, file offset will be 4 */
	SAFE_READ(cleanup, 1, fd, read_buf, 4);

	TEST(lseek64(fd, (loff_t) 1, SEEK_CUR));

	if (TEST_RETURN == (loff_t) -1) {
		tst_resm(TFAIL | TTERRNO, "llseek failed on %s ", TEST_FILE);
		goto cleanup_seekcur;
	}

	if (TEST_RETURN != 5) {
		tst_resm(TFAIL, "llseek return a incorrect file offset");
		goto cleanup_seekcur;
	}

	memset(read_buf, 0, sizeof(read_buf));

	/* the expected characters are "fgh" */
	SAFE_READ(cleanup, 1, fd, read_buf, 3);

	if (strcmp(read_buf, "fgh"))
		tst_resm(TFAIL, "Read wrong bytes after llseek");
	else
		tst_resm(TPASS, "test SEEK_SET for llseek success");

cleanup_seekcur:
	SAFE_CLOSE(cleanup, fd);
}


static void testfunc_seekend(void)
{
	int fd;
	ssize_t nread;
	static char read_buf[BUFSIZ];

	/* reopen TEST_FILE and file offset will be 0 */
	fd = SAFE_OPEN(cleanup, TEST_FILE, O_RDONLY);

	TEST(lseek64(fd, (loff_t) 0, SEEK_END));

	if (TEST_RETURN == (loff_t) -1) {
		tst_resm(TFAIL | TTERRNO, "llseek failed on %s ", TEST_FILE);
		goto cleanup_seekend;
	}

	if (TEST_RETURN != (long)file_size) {
		tst_resm(TFAIL, "llseek return a incorrect file offset");
		goto cleanup_seekend;
	}

	memset(read_buf, 0, sizeof(read_buf));

	nread = SAFE_READ(cleanup, 0, fd, read_buf, file_size);
	if (nread > 0)
		tst_resm(TFAIL, "Read bytes after llseek to end of file");
	else
		tst_resm(TPASS, "test SEEK_END for llseek success");

cleanup_seekend:
	SAFE_CLOSE(cleanup, fd);
}

static void cleanup(void)
{
	tst_rmdir();
}
