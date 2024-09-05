// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2024 SUSE LLC Andrea Cervesato <andrea.cervesato@suse.com>
 */

/*\
 * [Description]
 *
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
static uint64_t mnt_id;
static uint64_t mnt_id_dont_exist = -1;
static size_t buff_size;
static size_t buff_size_invalid = -1;

struct tcase {
	int exp_errno;
	char *msg;
	uint64_t *mnt_id;
	uint64_t mask;
	unsigned int flags;
	size_t *buff_size;
	struct statmount **buff;
} tcases[] = {
	{
		ENOENT,
		"mount id doesn't exist'",
		&mnt_id_dont_exist,
		0,
		0,
		&buff_size,
		&st_mount
	},
	{
		EOVERFLOW,
		"invalid mask",
		&mnt_id,
		-1,
		0,
		&buff_size,
		&st_mount
	},
	{
		EOVERFLOW,
		"small buffer for fs type",
		&mnt_id,
		STATMOUNT_FS_TYPE,
		0,
		&buff_size,
		&st_mount_small
	},
	{
		EOVERFLOW,
		"small buffer for mnt root",
		&mnt_id,
		STATMOUNT_MNT_ROOT,
		0,
		&buff_size,
		&st_mount_small
	},
	{
		EOVERFLOW,
		"small buffer for mnt point",
		&mnt_id,
		STATMOUNT_MNT_POINT,
		0,
		&buff_size,
		&st_mount_small
	},
	{
		EINVAL,
		"flags must be zero",
		&mnt_id,
		0,
		1,
		&buff_size,
		&st_mount
	},
	{
		EFAULT,
		"invalid buffer size",
		&mnt_id,
		0,
		0,
		&buff_size_invalid,
		&st_mount
	},
	{
		EFAULT,
		"invalid buffer pointer",
		&mnt_id,
		0,
		0,
		&buff_size,
		&st_mount_null
	},
};

static void run(unsigned int n)
{
	struct tcase *tc = &tcases[n];

	memset(st_mount, 0, sizeof(struct statmount));

	TST_EXP_FAIL(statmount(*tc->mnt_id, tc->mask,
		*tc->buff, *tc->buff_size, tc->flags),
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
	.format_device = 1,
	.bufs = (struct tst_buffers []) {
		{&st_mount, .size = sizeof(struct statmount)},
		{&st_mount_small, .size = sizeof(struct statmount)},
		{}
	}
};
