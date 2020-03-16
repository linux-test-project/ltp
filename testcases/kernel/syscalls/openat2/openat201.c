// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2020 Viresh Kumar <viresh.kumar@linaro.org>
 *
 * Basic openat2() test.
 */
#include "tst_test.h"
#include "lapi/openat2.h"

#define TEST_FILE "test_file"
#define TEST_DIR "test_dir"

static struct open_how *how;
static struct open_how_pad *phow;

static int dir_fd = -1, fd_atcwd = AT_FDCWD;

static struct tcase {
	int *dfd;
	const char *pathname;
	uint64_t flags;
	uint64_t mode;
	uint64_t resolve;
	struct open_how **how;
	size_t size;
} tcases[] = {
	{&dir_fd, TEST_FILE, O_RDWR, S_IRWXU, 0, &how, sizeof(*how)},
	{&dir_fd, TEST_FILE, O_RDONLY, S_IRUSR, 0, &how, sizeof(*how)},
	{&dir_fd, TEST_FILE, O_WRONLY, S_IWUSR, 0, &how, sizeof(*how)},
	{&dir_fd, TEST_FILE, O_RDWR, S_IRWXU, RESOLVE_NO_XDEV, &how, sizeof(*how)},
	{&dir_fd, TEST_FILE, O_RDWR, S_IRWXU, RESOLVE_NO_MAGICLINKS, &how, sizeof(*how)},
	{&dir_fd, TEST_FILE, O_RDWR, S_IRWXU, RESOLVE_NO_SYMLINKS, &how, sizeof(*how)},
	{&dir_fd, TEST_FILE, O_RDWR, S_IRWXU, RESOLVE_BENEATH, &how, sizeof(*how)},
	{&dir_fd, TEST_FILE, O_RDWR, S_IRWXU, RESOLVE_IN_ROOT, &how, sizeof(*how)},
	{&fd_atcwd, TEST_FILE, O_RDWR, S_IRWXU, 0, &how, sizeof(*how)},
	{&fd_atcwd, TEST_FILE, O_RDONLY, S_IRUSR, 0, &how, sizeof(*how)},
	{&fd_atcwd, TEST_FILE, O_WRONLY, S_IWUSR, 0, &how, sizeof(*how)},
	{&fd_atcwd, TEST_FILE, O_RDWR, S_IRWXU, RESOLVE_NO_XDEV, &how, sizeof(*how)},
	{&fd_atcwd, TEST_FILE, O_RDWR, S_IRWXU, RESOLVE_NO_MAGICLINKS, &how, sizeof(*how)},
	{&fd_atcwd, TEST_FILE, O_RDWR, S_IRWXU, RESOLVE_NO_SYMLINKS, &how, sizeof(*how)},
	{&fd_atcwd, TEST_FILE, O_RDWR, S_IRWXU, RESOLVE_BENEATH, &how, sizeof(*how)},
	{&fd_atcwd, TEST_FILE, O_RDWR, S_IRWXU, RESOLVE_IN_ROOT, (struct open_how **)&phow, sizeof(*how) + 8},
};

static void cleanup(void)
{
	if (dir_fd != -1)
		SAFE_CLOSE(dir_fd);
}

static void setup(void)
{
	openat2_supported_by_kernel();

	phow->pad = 0x00;
	SAFE_MKDIR(TEST_DIR, 0700);
	dir_fd = SAFE_OPEN(TEST_DIR, O_DIRECTORY);
}

static void run(unsigned int n)
{
	int fd;
	struct stat file_stat;
	struct tcase *tc = &tcases[n];
	struct open_how *myhow = *tc->how;

	myhow->flags = tc->flags | O_CREAT;
	myhow->mode = tc->mode;
	myhow->resolve = tc->resolve;

	TEST(fd = openat2(*tc->dfd, tc->pathname, myhow, tc->size));
	if (fd < 0) {
		tst_res(TFAIL | TTERRNO, "openat2() failed (%d)", n);
		return;
	}

	SAFE_FSTAT(fd, &file_stat);

	if (file_stat.st_size == 0)
		tst_res(TPASS, "openat2() passed (%d)", n);
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
	.bufs = (struct tst_buffers []) {
		{&how, .size = sizeof(*how)},
		{&phow, .size = sizeof(*phow)},
		{},
	},
};
