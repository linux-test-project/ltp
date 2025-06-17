// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2024 SUSE LLC Andrea Cervesato <andrea.cervesato@suse.com>
 */

/*\
 * Verify that listmount() raises the correct errors according with
 * invalid data:
 *
 * - EFAULT: req or mnt_id are unaccessible memories
 * - EINVAL: invalid flags or mnt_id request
 * - ENOENT: non-existent mount point
 */

#define _GNU_SOURCE

#include "tst_test.h"
#include "lapi/mount.h"
#include "lapi/syscalls.h"

#define MNT_SIZE 32

static struct mnt_id_req *request;
static uint64_t mnt_ids[MNT_SIZE];

static struct tcase {
	int req_usage;
	uint32_t size;
	uint32_t spare;
	uint64_t mnt_id;
	uint64_t param;
	uint64_t *mnt_ids;
	size_t nr_mnt_ids;
	uint64_t flags;
	int exp_errno;
	char *msg;
} tcases[] = {
	{
		.req_usage = 0,
		.mnt_ids = mnt_ids,
		.nr_mnt_ids = MNT_SIZE,
		.exp_errno = EFAULT,
		.msg = "request points to unaccessible memory",
	},
	{
		.req_usage = 1,
		.size = MNT_ID_REQ_SIZE_VER0,
		.mnt_id = LSMT_ROOT,
		.mnt_ids = NULL,
		.nr_mnt_ids = MNT_SIZE,
		.exp_errno = EFAULT,
		.msg = "mnt_ids points to unaccessible memory",
	},
	{
		.req_usage = 1,
		.size = MNT_ID_REQ_SIZE_VER0,
		.mnt_id = LSMT_ROOT,
		.mnt_ids = mnt_ids,
		.nr_mnt_ids = MNT_SIZE,
		.flags = -1,
		.exp_errno = EINVAL,
		.msg = "invalid flags",
	},
	{
		.req_usage = 1,
		.size = 0,
		.mnt_id = LSMT_ROOT,
		.mnt_ids = mnt_ids,
		.nr_mnt_ids = MNT_SIZE,
		.exp_errno = EINVAL,
		.msg = "insufficient mnt_id_req.size",
	},
	{
		.req_usage = 1,
		.size = MNT_ID_REQ_SIZE_VER0,
		.spare = -1,
		.mnt_id = LSMT_ROOT,
		.mnt_ids = mnt_ids,
		.nr_mnt_ids = MNT_SIZE,
		.exp_errno = EINVAL,
		.msg = "invalid mnt_id_req.spare",
	},
	{
		.req_usage = 1,
		.size = MNT_ID_REQ_SIZE_VER0,
		.mnt_id = LSMT_ROOT,
		.param = STATMOUNT_PROPAGATE_FROM + 1,
		.mnt_ids = mnt_ids,
		.nr_mnt_ids = MNT_SIZE,
		.exp_errno = EINVAL,
		.msg = "invalid mnt_id_req.param",
	},
	{
		.req_usage = 1,
		.size = MNT_ID_REQ_SIZE_VER0,
		.mnt_id = 0,
		.mnt_ids = mnt_ids,
		.nr_mnt_ids = MNT_SIZE,
		.exp_errno = EINVAL,
		.msg = "invalid mnt_id_req.mnt_id",
	},
	{
		.req_usage = 1,
		.size = MNT_ID_REQ_SIZE_VER0,
		.mnt_id = LSMT_ROOT - 1,
		.mnt_ids = mnt_ids,
		.nr_mnt_ids = MNT_SIZE,
		.exp_errno = ENOENT,
		.msg = "non-existant mnt_id",
	},
};

static void run(unsigned int n)
{
	struct tcase *tc = &tcases[n];
	struct mnt_id_req *req = NULL;

	memset(mnt_ids, 0, sizeof(mnt_ids));

	if (tc->req_usage) {
		req = request;
		req->mnt_id = tc->mnt_id;
		req->param = tc->param;
		req->size = tc->size;
		req->spare = tc->spare;
	}

	TST_EXP_FAIL(tst_syscall(__NR_listmount, req, tc->mnt_ids,
		tc->nr_mnt_ids, tc->flags), tc->exp_errno,
		"%s", tc->msg);
}

static struct tst_test test = {
	.test = run,
	.tcnt = ARRAY_SIZE(tcases),
	.min_kver = "6.8",
	.bufs = (struct tst_buffers []) {
		{ &request, .size = MNT_ID_REQ_SIZE_VER0 },
		{},
	},
};
