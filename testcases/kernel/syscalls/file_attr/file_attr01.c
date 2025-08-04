// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2025 SUSE LLC Andrea Cervesato <andrea.cervesato@suse.com>
 */

/*\
 * Verify that `file_getattr` and `file_setattr` syscalls are raising the
 * correct errors according to the invalid input arguments. In particular:
 *
 * - EBADFD: Invalid file descriptor.
 * - ENOENT: File doesn't exist
 * - EFAULT: File name is NULL
 * - EFAULT: File attributes is NULL
 * - EINVAL: File attributes size is zero
 * - E2BIG: File attributes size is too big
 * - EINVAL: Invalid AT flags
 */

#include <string.h>
#include "tst_test.h"
#include "lapi/fs.h"
#include "lapi/fcntl.h"

#define MNTPOINT "mntpoint"
#define FILENAME "ltp_file"
#define NO_FILENAME "ltp_file_doesnt_exist"

static int valid_dfd = -1;
static int invalid_dfd = -1;
static char *valid_filename;
static char *invalid_filename;
static char *null_ptr;
static size_t zero;
static size_t small_usize;
static size_t valid_usize;
static size_t big_usize;
static struct file_attr *valid_file_attr;

static struct tcase {
	int *dfd;
	char **filename;
	struct file_attr **ufattr;
	size_t *usize;
	int at_flags;
	int exp_errno;
	char *msg;
} tcases[] = {
	{
		.dfd = &invalid_dfd,
		.filename = &valid_filename,
		.ufattr = &valid_file_attr,
		.usize = &valid_usize,
		.exp_errno = EBADF,
		.msg = "Invalid file descriptor",
	},
	{
		.dfd = &valid_dfd,
		.filename = &invalid_filename,
		.ufattr = &valid_file_attr,
		.usize = &valid_usize,
		.exp_errno = ENOENT,
		.msg = "File doesn't exist",
	},
	{
		.dfd = &valid_dfd,
		.filename = &null_ptr,
		.ufattr = &valid_file_attr,
		.usize = &valid_usize,
		.exp_errno = EFAULT,
		.msg = "Filename is NULL",
	},
	{
		.dfd = &valid_dfd,
		.filename = &valid_filename,
		.ufattr = (struct file_attr **)(&null_ptr),
		.usize = &valid_usize,
		.exp_errno = EFAULT,
		.msg = "File attributes is NULL",
	},
	{
		.dfd = &valid_dfd,
		.filename = &valid_filename,
		.ufattr = &valid_file_attr,
		.usize = &zero,
		.exp_errno = EINVAL,
		.msg = "File attributes size is zero",
	},
	{
		.dfd = &valid_dfd,
		.filename = &valid_filename,
		.ufattr = &valid_file_attr,
		.usize = &small_usize,
		.exp_errno = EINVAL,
		.msg = "File attributes size is too small",
	},
	{
		.dfd = &valid_dfd,
		.filename = &valid_filename,
		.ufattr = &valid_file_attr,
		.usize = &big_usize,
		.exp_errno = E2BIG,
		.msg = "File attributes size is too big",
	},
	{
		.dfd = &valid_dfd,
		.filename = &valid_filename,
		.ufattr = &valid_file_attr,
		.usize = &valid_usize,
		.at_flags = -1,
		.exp_errno = EINVAL,
		.msg = "Invalid AT flags",
	},
};

static void run(unsigned int i)
{
	struct tcase *tc = &tcases[i];

	if (tst_variant) {
		TST_EXP_FAIL(file_getattr(
			*tc->dfd, *tc->filename,
			*tc->ufattr, *tc->usize,
			tc->at_flags),
			tc->exp_errno,
			"%s", tc->msg);
	} else {
		TST_EXP_FAIL(file_setattr(
			*tc->dfd, *tc->filename,
			*tc->ufattr, *tc->usize,
			tc->at_flags),
			tc->exp_errno,
			"%s", tc->msg);
	}
}

static void setup(void)
{
	valid_dfd = SAFE_OPEN(MNTPOINT, O_RDONLY);

	SAFE_CHDIR(MNTPOINT);
	SAFE_TOUCH(FILENAME, 0777, NULL);
	SAFE_CHDIR("..");

	valid_usize = FILE_ATTR_SIZE_LATEST;
	small_usize = FILE_ATTR_SIZE_VER0 - 1;
	big_usize = SAFE_SYSCONF(_SC_PAGESIZE) + 100;
}

static void cleanup(void)
{
	if (valid_dfd != -1)
		SAFE_CLOSE(valid_dfd);
}

static struct tst_test test = {
	.test = run,
	.setup = setup,
	.cleanup = cleanup,
	.tcnt = ARRAY_SIZE(tcases),
	.mntpoint = MNTPOINT,
	.needs_root = 1,
	.mount_device = 1,
	.all_filesystems = 1,
	.test_variants = 2,
	.skip_filesystems = (const char *const []) {
		"fuse",
		"ntfs",
		"vfat",
		"exfat",
		NULL
	},
	.bufs = (struct tst_buffers []) {
		{&valid_filename, .str = FILENAME},
		{&invalid_filename, .str = NO_FILENAME},
		{&valid_file_attr, .size = sizeof(struct file_attr)},
		{}
	}
};
