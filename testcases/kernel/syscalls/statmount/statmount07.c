// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2024 SUSE LLC Andrea Cervesato <andrea.cervesato@suse.com>
 */

/*\
 * This test verifies that statmount() is raising the correct errors according
 * with invalid input values.
 */

#define _GNU_SOURCE

#include "statmount.h"
#include "lapi/stat.h"

#define MNTPOINT "mntpoint"

static struct statmount *st_mount;
static struct statmount *st_mount_null;
static struct statmount *st_mount_small;
static struct statmount *st_mount_bad;
static uint64_t mnt_id;
static uint64_t mnt_id_dont_exist = -1;
static size_t buff_size;

struct tcase {
	int exp_errno;
	char *msg;
	uint64_t *mnt_id;
	uint64_t mask;
	unsigned int flags;
	struct statmount **buff;
} tcases[] = {
	{
		.exp_errno = ENOENT,
		.msg = "mount id doesn't exist'",
		.mnt_id = &mnt_id_dont_exist,
		.buff = &st_mount
	},
	{
		.exp_errno = EOVERFLOW,
		.msg = "invalid mask",
		.mnt_id = &mnt_id,
		.mask = -1,
		.buff = &st_mount
	},
	{
		.exp_errno = EOVERFLOW,
		.msg = "small buffer for fs type",
		.mnt_id = &mnt_id,
		.mask = STATMOUNT_FS_TYPE,
		.buff = &st_mount_small,
	},
	{
		.exp_errno = EOVERFLOW,
		.msg = "small buffer for mnt root",
		.mnt_id = &mnt_id,
		.mask = STATMOUNT_MNT_ROOT,
		.buff = &st_mount_small,
	},
	{
		.exp_errno = EOVERFLOW,
		.msg = "small buffer for mnt point",
		.mnt_id = &mnt_id,
		.mask = STATMOUNT_MNT_POINT,
		.buff = &st_mount_small,
	},
	{
		.exp_errno = EINVAL,
		.msg = "flags must be zero",
		.mnt_id = &mnt_id,
		.flags = 1,
		.buff = &st_mount,
	},
	{
		.exp_errno = EFAULT,
		.msg = "buffer crosses to PROT_NONE",
		.mnt_id = &mnt_id,
		.buff = &st_mount_bad,
	},
	{
		.exp_errno = EFAULT,
		.msg = "invalid buffer pointer",
		.mnt_id = &mnt_id,
		.buff = &st_mount_null,
	},
};

static void run(unsigned int n)
{
	struct tcase *tc = &tcases[n];

	memset(st_mount, 0, sizeof(struct statmount));

	TST_EXP_FAIL(statmount(*tc->mnt_id, tc->mask,
		*tc->buff, buff_size, tc->flags),
		tc->exp_errno, "%s", tc->msg);
}

static void setup(void)
{
	struct ltp_statx sx;

	SAFE_STATX(AT_FDCWD, MNTPOINT, 0, STATX_MNT_ID_UNIQUE, &sx);

	mnt_id = sx.data.stx_mnt_id;
	buff_size = sizeof(struct statmount);
}
static struct tst_test test = {
	.test = run,
	.tcnt = ARRAY_SIZE(tcases),
	.setup = setup,
	.min_kver = "6.8",
	.mount_device = 1,
	.mntpoint = MNTPOINT,
	.bufs = (struct tst_buffers []) {
		{&st_mount, .size = sizeof(struct statmount)},
		{&st_mount_small, .size = sizeof(struct statmount)},
		{&st_mount_bad, .size = 1},
		{}
	}
};
