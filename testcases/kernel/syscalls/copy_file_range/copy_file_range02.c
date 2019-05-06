// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2019 SUSE LLC
 * Author: Christian Amann <camann@suse.com>
 */

/*
 * Tests basic error handling of the
 * copy_file_range syscall
 *
 * 1) Try to copy contents to file open as readonly
 *    -> EBADF
 * 2) Try to copy contents to file on different mounted
 *    filesystem -> EXDEV
 * 3) Try to copy contents to directory -> EISDIR
 * 4) Try to copy contents to a file opened with the
 *    O_APPEND flag -> EBADF
 * 5) Try to copy contents to closed filedescriptor
 *    -> EBADF
 * 6) Try to copy contents with invalid 'flags' value
 *    -> EINVAL
 */

#define _GNU_SOURCE

#include "tst_test.h"
#include "copy_file_range.h"

static int fd_src;
static int fd_dest;
static int fd_rdonly;
static int fd_mnted;
static int fd_dir;
static int fd_closed;
static int fd_append;

static struct tcase {
	int	*copy_to_fd;
	int	flags;
	int	exp_err;
} tcases[] = {
	{&fd_rdonly,	0,	EBADF},
	{&fd_mnted,	0,	EXDEV},
	{&fd_dir,	0,	EISDIR},
	{&fd_append,	0,	EBADF},
	{&fd_closed,	0,	EBADF},
	{&fd_dest,	-1,	EINVAL},
};

static void verify_copy_file_range(unsigned int n)
{
	struct tcase *tc = &tcases[n];

	TEST(sys_copy_file_range(fd_src, 0, *tc->copy_to_fd,
				0, CONTSIZE, tc->flags));

	if (TST_RET == -1) {
		if (tc->exp_err == TST_ERR) {
			tst_res(TPASS | TTERRNO,
					"copy_file_range failed as expected");
		} else {
			tst_res(TFAIL | TTERRNO,
				"copy_file_range failed unexpectedly; expected %s, but got",
				tst_strerrno(tc->exp_err));
			return;
		}
	} else {
		tst_res(TFAIL,
			"copy_file_range returned wrong value: %ld", TST_RET);
	}
}

static void cleanup(void)
{
	if (fd_append > 0)
		SAFE_CLOSE(fd_append);
	if (fd_dir > 0)
		SAFE_CLOSE(fd_dir);
	if (fd_mnted > 0)
		SAFE_CLOSE(fd_mnted);
	if (fd_rdonly > 0)
		SAFE_CLOSE(fd_rdonly);
	if (fd_dest > 0)
		SAFE_CLOSE(fd_dest);
	if (fd_src > 0)
		SAFE_CLOSE(fd_src);
}

static void setup(void)
{
	syscall_info();

	if (access(FILE_DIR_PATH, F_OK) == -1)
		SAFE_MKDIR(FILE_DIR_PATH, 0777);

	fd_src    = SAFE_OPEN(FILE_SRC_PATH, O_RDWR | O_CREAT, 0664);
	fd_dest   = SAFE_OPEN(FILE_DEST_PATH, O_RDWR | O_CREAT, 0664);
	fd_rdonly = SAFE_OPEN(FILE_RDONL_PATH, O_RDONLY | O_CREAT, 0664);
	fd_mnted  = SAFE_OPEN(FILE_MNTED_PATH, O_RDWR | O_CREAT, 0664);
	fd_dir    = SAFE_OPEN(FILE_DIR_PATH, O_DIRECTORY);
	fd_closed = -1;
	fd_append = SAFE_OPEN(FILE_DEST_PATH,
			O_RDWR | O_CREAT | O_APPEND, 0664);

	SAFE_WRITE(1, fd_src,  CONTENT,  CONTSIZE);
}

static struct tst_test test = {
	.test = verify_copy_file_range,
	.tcnt = ARRAY_SIZE(tcases),
	.setup = setup,
	.cleanup = cleanup,
	.needs_root = 1,
	.mount_device = 1,
	.mntpoint = MNTPOINT,
	.dev_fs_type = "ext4",
	.test_variants = TEST_VARIANTS,
};
