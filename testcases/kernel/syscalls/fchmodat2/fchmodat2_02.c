// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2024 SUSE LLC Andrea Cervesato <andrea.cervesato@suse.com>
 */

/*\
 * This test verifies that fchmodat2() syscall properly raises errors with
 * invalid values.
 */

#include "tst_test.h"
#include "lapi/fcntl.h"
#include "lapi/syscalls.h"
#include "tst_tmpdir.h"

#define FILENAME "file.bin"

static char *tmpdir;
static int fd;
static int fd_invalid = -1;

static struct tcase {
	int *fd;
	char *fname;
	int mode;
	int flag;
	int exp_errno;
	char *msg;
} tcases[] = {
	{&fd_invalid, FILENAME, 0777, 0, EBADF, "bad file descriptor"},
	{&fd, "doesnt_exist.txt", 0777, 0, ENOENT, "unexisting file"},
	{&fd, FILENAME, 0777, -1, EINVAL, "invalid flags"},
};

static void run(unsigned int i)
{
	struct tcase *tc = &tcases[i];

	TST_EXP_FAIL(tst_syscall(__NR_fchmodat2,
		*tc->fd, tc->fname, tc->mode, tc->flag),
		tc->exp_errno,
		"Test %s", tc->msg);
}

static void setup(void)
{
	tmpdir = tst_tmpdir_path();

	SAFE_TOUCH(FILENAME, 0640, NULL);
	fd = SAFE_OPEN(tmpdir, O_PATH | O_DIRECTORY, 0640);
}

static void cleanup(void)
{
	if (fd != -1)
		SAFE_CLOSE(fd);
}

static struct tst_test test = {
	.test = run,
	.tcnt = ARRAY_SIZE(tcases),
	.setup = setup,
	.cleanup = cleanup,
	.needs_tmpdir = 1,
};

