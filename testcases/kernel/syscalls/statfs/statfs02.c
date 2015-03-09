/*
 * Copyright (c) International Business Machines  Corp., 2001
 *	07/2001 Ported by Wayne Boyer
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
 * DESCRIPTION
 *	1.	Use a component of the pathname, which is not a directory
 *		in the "path" parameter to statfs(). Expect ENOTDIR
 *	2.	Pass a filename which doesn't exist, and expect ENOENT.
 *	3.	Pass a pathname which is more than MAXNAMLEN, and expect
 *		ENAMETOOLONG.
 *	4.	Pass a pointer to the pathname outside the address space of
 *		the process, and expect EFAULT.
 *	5.	Pass a pointer to the buf paramter outside the address space
 *		of the process, and expect EFAULT.
 *	6.	Pass a filename which has too many symbolic links, and expect
 *		ELOOP.
 */

#include <sys/types.h>
#include <sys/statfs.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/vfs.h>
#include <sys/mman.h>
#include <errno.h>
#include "test.h"
#include "safe_macros.h"

char *TCID = "statfs02";

static int fd;

#define TEST_FILE		"statfs_file"
#define TEST_FILE1		TEST_FILE"/statfs_file1"
#define TEST_NOEXIST		"statfs_noexist"
#define TEST_SYMLINK		"statfs_symlink"

static char test_toolong[PATH_MAX+2];
static struct statfs buf;

static struct test_case_t {
	char *path;
	struct statfs *buf;
	int exp_error;
} TC[] = {
	{TEST_FILE1, &buf, ENOTDIR},
	{TEST_NOEXIST, &buf, ENOENT},
	{test_toolong, &buf, ENAMETOOLONG},
#ifndef UCLINUX
	{(char *)-1, &buf, EFAULT},
	{TEST_FILE, (struct statfs *)-1, EFAULT},
#endif
	{TEST_SYMLINK, &buf, ELOOP},
};

int TST_TOTAL = ARRAY_SIZE(TC);
static void setup(void);
static void cleanup(void);
static void statfs_verify(const struct test_case_t *);

int main(int ac, char **av)
{
	int lc;
	int i;

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		tst_count = 0;
		for (i = 0; i < TST_TOTAL; i++)
			statfs_verify(&TC[i]);
	}

	cleanup();
	tst_exit();
}

static void setup(void)
{
	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;

	tst_tmpdir();

	fd = SAFE_CREAT(cleanup, TEST_FILE, 0444);

	memset(test_toolong, 'a', PATH_MAX+1);

#if !defined(UCLINUX)
	TC[3].path = SAFE_MMAP(cleanup, 0, 1, PROT_NONE,
			       MAP_PRIVATE | MAP_ANONYMOUS, 0, 0);
#endif

	SAFE_SYMLINK(cleanup, TEST_SYMLINK, "statfs_symlink_2");
	SAFE_SYMLINK(cleanup, "statfs_symlink_2", TEST_SYMLINK);
}

static void statfs_verify(const struct test_case_t *test)
{
	TEST(statfs(test->path, test->buf));

	if (TEST_RETURN != -1) {
		tst_resm(TFAIL, "call succeeded unexpectedly");
		return;
	}

	if (TEST_ERRNO == test->exp_error) {
		tst_resm(TPASS | TTERRNO, "expected failure");
	} else {
		tst_resm(TFAIL | TTERRNO, "unexpected error, expected %d",
			 TEST_ERRNO);
	}
}

static void cleanup(void)
{
	if (fd > 0)
		close(fd);

	tst_rmdir();
}
