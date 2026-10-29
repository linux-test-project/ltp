// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2020 Viresh Kumar <viresh.kumar@linaro.org>
 */

/*\
 * Basic name_to_handle_at() tests.
 *
 * [Algorithm]
 *  - Check that EOVERFLOW is returned as expected by name_to_handle_at().
 *  - Check that we were able to access a file's stat with name_to_handle_at()
 *    and open_by_handle_at().
 */

#define _GNU_SOURCE
#include <sys/stat.h>
#include "lapi/name_to_handle_at.h"

#define TEST_FILE "test_file"
#define TEST_DIR "test_dir"

static int dir_fd, fd_atcwd = AT_FDCWD, file_fd;
static struct file_handle *fhp;

static struct tcase {
	int *dfd;
	const char *pathname;
	int name_flags;
	int flags;
} tcases[] = {
	{&dir_fd, TEST_FILE, 0, O_RDWR},
	{&dir_fd, TEST_FILE, 0, O_RDONLY},
	{&dir_fd, TEST_FILE, 0, O_WRONLY},
	{&dir_fd, TEST_FILE, AT_EMPTY_PATH, O_RDWR},
	{&dir_fd, TEST_FILE, AT_EMPTY_PATH, O_RDONLY},
	{&dir_fd, TEST_FILE, AT_EMPTY_PATH, O_WRONLY},
	{&dir_fd, TEST_FILE, AT_SYMLINK_FOLLOW, O_RDWR},
	{&dir_fd, TEST_FILE, AT_SYMLINK_FOLLOW, O_RDONLY},
	{&dir_fd, TEST_FILE, AT_SYMLINK_FOLLOW, O_WRONLY},
	{&dir_fd, TEST_FILE, AT_EMPTY_PATH | AT_SYMLINK_FOLLOW, O_RDWR},
	{&dir_fd, TEST_FILE, AT_EMPTY_PATH | AT_SYMLINK_FOLLOW, O_RDONLY},
	{&dir_fd, TEST_FILE, AT_EMPTY_PATH | AT_SYMLINK_FOLLOW, O_WRONLY},
	{&dir_fd, "", AT_EMPTY_PATH, O_RDONLY},
	{&file_fd, "", AT_EMPTY_PATH, O_RDONLY},

	{&fd_atcwd, TEST_FILE, 0, O_RDWR},
	{&fd_atcwd, TEST_FILE, 0, O_RDONLY},
	{&fd_atcwd, TEST_FILE, 0, O_WRONLY},
	{&fd_atcwd, TEST_FILE, AT_EMPTY_PATH, O_RDWR},
	{&fd_atcwd, TEST_FILE, AT_EMPTY_PATH, O_RDONLY},
	{&fd_atcwd, TEST_FILE, AT_EMPTY_PATH, O_WRONLY},
	{&fd_atcwd, TEST_FILE, AT_SYMLINK_FOLLOW, O_RDWR},
	{&fd_atcwd, TEST_FILE, AT_SYMLINK_FOLLOW, O_RDONLY},
	{&fd_atcwd, TEST_FILE, AT_SYMLINK_FOLLOW, O_WRONLY},
	{&fd_atcwd, TEST_FILE, AT_EMPTY_PATH | AT_SYMLINK_FOLLOW, O_RDWR},
	{&fd_atcwd, TEST_FILE, AT_EMPTY_PATH | AT_SYMLINK_FOLLOW, O_RDONLY},
	{&fd_atcwd, TEST_FILE, AT_EMPTY_PATH | AT_SYMLINK_FOLLOW, O_WRONLY},
	{&fd_atcwd, "", AT_EMPTY_PATH, O_RDONLY},
};

static void cleanup(void)
{
	SAFE_CLOSE(dir_fd);
	SAFE_CLOSE(file_fd);
}

static void setup(void)
{
	SAFE_MKDIR(TEST_DIR, 0700);
	dir_fd = SAFE_OPEN(TEST_DIR, O_DIRECTORY);
	SAFE_CHDIR(TEST_DIR);
	SAFE_TOUCH(TEST_FILE, 0600, NULL);
	file_fd = SAFE_OPEN("foo_file", O_RDWR | O_CREAT, 0600);
	fhp = allocate_file_handle(AT_FDCWD, TEST_FILE);
}

static void run(unsigned int n)
{
	struct tcase *tc = &tcases[n];
	struct stat file_stat;
	int fd, mount_id;

	TEST(name_to_handle_at(*tc->dfd, tc->pathname, fhp, &mount_id,
			       tc->name_flags));
	if (TST_RET) {
		tst_res(TFAIL | TTERRNO, "name_to_handle_at() failed (%d)", n);
		return;
	}

	TEST(fd = open_by_handle_at(*tc->dfd, fhp, tc->flags));
	if (fd < 0) {
		tst_res(TFAIL | TTERRNO, "open_by_handle_at() failed (%d)", n);
		return;
	}

	SAFE_FSTAT(fd, &file_stat);

	/* Don't check stats when pathname is empty */
	if (file_stat.st_size == 0 || !tc->pathname[0])
		tst_res(TPASS, "name_to_handle_at() passed (%d)", n);
	else
		tst_res(TFAIL, "fstat() didn't work as expected (%d)", n);

	SAFE_CLOSE(fd);
}

static struct tst_test test = {
	.tcnt = ARRAY_SIZE(tcases),
	.test = run,
	.setup = setup,
	.cleanup = cleanup,
	.needs_tmpdir = 1,
	.needs_root = 1,
};
