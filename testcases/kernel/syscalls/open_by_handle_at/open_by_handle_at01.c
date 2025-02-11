// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2020 Viresh Kumar <viresh.kumar@linaro.org>
 */

/*\
 * Basic open_by_handle_at() tests.
 *
 * [Algorithm]
 *
 * Check that we were able to access a file's stat which is opened with
 * open_by_handle_at().
 */

#define _GNU_SOURCE
#include <sys/stat.h>
#include "lapi/name_to_handle_at.h"

#define TEST_FILE "test_file"
#define TEST_DIR "test_dir"

static int dir_fd, fd_atcwd = AT_FDCWD, file_fd;
static struct file_handle *f_fhp, *d_fhp, *at_fhp;
static struct file_handle *f_fhp, *d_fhp, *at_fhp;

static struct tcase {
	int *dfd;
	struct file_handle **fhp;
	int flags;
} tcases[] = {
	{&dir_fd, &d_fhp, O_RDWR},
	{&dir_fd, &d_fhp, O_RDONLY},
	{&dir_fd, &d_fhp, O_WRONLY},
	{&file_fd, &f_fhp, O_RDWR},
	{&file_fd, &f_fhp, O_RDONLY},
	{&file_fd, &f_fhp, O_WRONLY},
	{&fd_atcwd, &at_fhp, O_RDWR},
	{&fd_atcwd, &at_fhp, O_RDONLY},
	{&fd_atcwd, &at_fhp, O_WRONLY},
};

static void cleanup(void)
{
	SAFE_CLOSE(dir_fd);
	SAFE_CLOSE(file_fd);
}

static void setup(void)
{
	int mount_id;

	SAFE_MKDIR(TEST_DIR, 0700);
	dir_fd = SAFE_OPEN(TEST_DIR, O_DIRECTORY);
	SAFE_CHDIR(TEST_DIR);
	SAFE_TOUCH(TEST_FILE, 0600, NULL);
	file_fd = SAFE_OPEN("foo_file", O_RDWR | O_CREAT, 0600);

	f_fhp = allocate_file_handle(AT_FDCWD, TEST_FILE);
	d_fhp = allocate_file_handle(AT_FDCWD, TEST_FILE);
	at_fhp = allocate_file_handle(AT_FDCWD, TEST_FILE);

	TEST(name_to_handle_at(file_fd, "", f_fhp, &mount_id, AT_EMPTY_PATH));
	if (TST_RET) {
		tst_res(TFAIL | TTERRNO, "name_to_handle_at() failed");
		return;
	}

	TEST(name_to_handle_at(dir_fd, TEST_FILE, d_fhp, &mount_id,
			       AT_EMPTY_PATH | AT_SYMLINK_FOLLOW));
	if (TST_RET) {
		tst_res(TFAIL | TTERRNO, "name_to_handle_at() failed");
		return;
	}

	TEST(name_to_handle_at(AT_FDCWD, TEST_FILE, at_fhp, &mount_id,
			       AT_EMPTY_PATH | AT_SYMLINK_FOLLOW));
	if (TST_RET) {
		tst_res(TFAIL | TTERRNO, "name_to_handle_at() failed");
		return;
	}
}

static void run(unsigned int n)
{
	struct tcase *tc = &tcases[n];
	struct stat file_stat;
	int fd;

	TEST(fd = open_by_handle_at(*tc->dfd, *tc->fhp, tc->flags));
	if (fd < 0) {
		tst_res(TFAIL | TTERRNO, "open_by_handle_at() failed (%d)", n);
		return;
	}

	SAFE_FSTAT(fd, &file_stat);

	/* Don't check stats when pathname is empty */
	if (file_stat.st_size == 0 || (tc->fhp == &f_fhp))
		tst_res(TPASS, "open_by_handle_at() passed (%d)", n);
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
