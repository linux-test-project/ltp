// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2019 SUSE LLC
 * Author: Christian Amann <camann@suse.com>
 */

/*\
 * [Description]
 *
 * Tests the ioctl functionality to deduplicate fileranges using
 * btrfs filesystem.
 *
 * 1. Sets the same contents for two files and deduplicates it.
 *	Deduplicates 3 bytes and set the status to
 *	FILE_DEDUPE_RANGE_SAME.
 * 2. Sets different content for two files and tries to
 *	deduplicate it. 0 bytes get deduplicated and status is
 *	set to FILE_DEDUPE_RANGE_DIFFERS.
 * 3. Sets same content for two files but sets the length to
 *	deduplicate to -1. ioctl(FIDEDUPERANGE) fails with EINVAL.
 */

#include "config.h"
#include <stdlib.h>
#include <sys/ioctl.h>
#include <errno.h>
#include "tst_test.h"

#ifdef HAVE_STRUCT_FILE_DEDUPE_RANGE
#include <linux/fs.h>

#define SUCCESS 0

#define MNTPOINT "mnt_point"
#define FILE_SRC_PATH	MNTPOINT"/file_src"
#define FILE_DEST_PATH	MNTPOINT"/file_dest"

static int fd_src;
static int fd_dest;
static struct file_dedupe_range *fdr;

static struct tcase {
	uint64_t	src_length;
	char		*src_fcontents;
	char		*dest_fcontents;
	int		exp_err;
	uint64_t	bytes_deduped;
	int		status;
} tcases[] = {
	{3, "AAA", "AAA", SUCCESS, 3, FILE_DEDUPE_RANGE_SAME},
	{3, "AAA", "AAB", SUCCESS, 0, FILE_DEDUPE_RANGE_DIFFERS},
	{-1, "AAA", "AAA", EINVAL, 0, 0},
};

static void verify_ioctl(unsigned int n)
{
	struct tcase *tc = &tcases[n];

	fd_src  = SAFE_OPEN(FILE_SRC_PATH,  O_RDWR | O_CREAT, 0664);
	fd_dest = SAFE_OPEN(FILE_DEST_PATH, O_RDWR | O_CREAT, 0664);

	SAFE_WRITE(SAFE_WRITE_ALL, fd_src,  tc->src_fcontents,  strlen(tc->src_fcontents));
	SAFE_WRITE(SAFE_WRITE_ALL, fd_dest, tc->dest_fcontents, strlen(tc->dest_fcontents));

	memset(fdr, 0, sizeof(struct file_dedupe_range) +
			sizeof(struct file_dedupe_range_info));

	fdr->src_length = tc->src_length;
	fdr->dest_count = 1;
	fdr->info[0].dest_fd = fd_dest;

	TEST(ioctl(fd_src, FIDEDUPERANGE, fdr));

	if (tc->exp_err != TST_ERR) {
		tst_res(TFAIL,
			"ioctl(FIDEDUPERANGE) ended with %s, expected %s",
			tst_strerrno(TST_ERR), tst_strerrno(tc->exp_err));
		return;
	}

	if (fdr->info[0].bytes_deduped != tc->bytes_deduped) {
		tst_res(TFAIL,
			"ioctl(FIDEDUPERANGE) deduplicated %lld bytes, expected %ld. Status: %d",
			fdr->info[0].bytes_deduped, tc->bytes_deduped,
			fdr->info[0].status);
		return;
	}

	if (fdr->info[0].status != tc->status) {
		tst_res(TFAIL,
			"ioctl(FIDEDUPERANGE) status set to %d, expected %d",
			fdr->info[0].status, tc->status);
		return;
	}

	tst_res(TPASS, "ioctl(FIDEDUPERANGE) ended with %s as expected",
			tst_strerrno(tc->exp_err));

	SAFE_CLOSE(fd_dest);
	SAFE_CLOSE(fd_src);
}

static void cleanup(void)
{
	if (fd_dest > 0)
		SAFE_CLOSE(fd_dest);
	if (fd_src > 0)
		SAFE_CLOSE(fd_src);
	if (fdr)
		free(fdr);
}

static void setup(void)
{
	fdr = SAFE_MALLOC(sizeof(struct file_dedupe_range) +
			sizeof(struct file_dedupe_range_info));
}


static struct tst_test test = {
	.test = verify_ioctl,
	.tcnt = ARRAY_SIZE(tcases),
	.setup = setup,
	.cleanup = cleanup,
	.min_kver = "4.5",
	.needs_root = 1,
	.mount_device = 1,
	.mntpoint = MNTPOINT,
	.dev_fs_type = "btrfs",
	.needs_drivers = (const char *const[]) {
		"btrfs",
		NULL,
	},
};
#else
	TST_TEST_TCONF(
		"This system does not provide support for ioctl(FIDEDUPERANGE)");
#endif
