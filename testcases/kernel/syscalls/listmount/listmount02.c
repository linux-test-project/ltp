// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2024 SUSE LLC Andrea Cervesato <andrea.cervesato@suse.com>
 */

/*\
 * This test verifies that listmount() is properly reading groups of mount IDs,
 * checking that both oneshoot and iterator API for listmount() return the same
 * array.
 *
 * [Algorithm]
 *
 * - move into a new unshared namespace
 * - mount() our new root inside temporary folder
 * - generate a full mounts tree inside root folder, doubling the number of
 *   mounted filesystems each bind mount
 * - read the full list of mounted IDs using listmount(LSMT_ROOT, ..)
 * - read the list of mounted IDs using groups of fixed size
 * - compare the first mount list with the second mount list
 */

#include "listmount.h"
#include "lapi/sched.h"

#define MNTPOINT "mntpoint"
#define BIND_MOUNTS 7
#define GROUPS_SIZE 3
#define LISTSIZE (1 << BIND_MOUNTS)

static void run(void)
{
	ssize_t ret;
	size_t id, tot_ids, count = 0;
	uint64_t mount_ids[LISTSIZE];
	uint64_t list[LISTSIZE];

	for (int i = 0; i < BIND_MOUNTS; i++)
		SAFE_MOUNT("/", "/", NULL, MS_BIND, NULL);

	tst_res(TINFO, "Reading all %d mount IDs in once", LISTSIZE);

	TST_EXP_POSITIVE(listmount(LSMT_ROOT, 0, mount_ids, LISTSIZE, 0));
	if (!TST_PASS)
		goto end;

	tot_ids = (size_t)TST_RET;

	if (tot_ids != LISTSIZE) {
		tst_res(TFAIL, "listmount() returned %lu but %d was expected",
			tot_ids, LISTSIZE);
		goto end;
	}

	tst_res(TINFO, "Reading groups of %d mount IDs", GROUPS_SIZE);

	while (count < LISTSIZE) {
		id = count ? list[count - 1] : 0;
		ret = listmount(LSMT_ROOT, id, list + count, GROUPS_SIZE, 0);

		tst_res(TDEBUG, "listmount(LSMT_ROOT, %lu, list + %lu, %d, 0)",
			id, count, GROUPS_SIZE);

		if (ret == -1) {
			tst_res(TFAIL, "listmount() failed with %s", tst_strerrno(errno));
			goto end;
		}

		count += ret;

		if (TST_RET < GROUPS_SIZE)
			break;
	}

	for (size_t i = 0; i < LISTSIZE; i++) {
		if (mount_ids[i] != list[i]) {
			tst_res(TFAIL, "Mount ID differs at %ld index", i);
			goto end;
		}
	}

	tst_res(TPASS, "All mount IDs have been correctly read");

end:
	for (int i = 0; i < BIND_MOUNTS; i++)
		SAFE_UMOUNT("/");
}

static void setup(void)
{
	SAFE_UNSHARE(CLONE_NEWNS);
	SAFE_CHROOT(MNTPOINT);

	SAFE_MOUNT("", "/", NULL, MS_REC | MS_SHARED, NULL);
}

static struct tst_test test = {
	.test_all = run,
	.setup = setup,
	.forks_child = 1,
	.min_kver = "6.8",
	.mount_device = 1,
	.mntpoint = MNTPOINT,
};
