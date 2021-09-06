// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2019 SUSE LLC
 * Author: Christian Amann <camann@suse.com>
 */

/*
 * Copies the contents of one file into another and
 * checks if the timestamp gets updated in the process.
 */

#define _GNU_SOURCE

#include "tst_test.h"
#include "tst_timer.h"
#include "copy_file_range.h"

static int fd_src;
static int fd_dest;

struct timespec get_timestamp(int fd)
{
	struct stat filestat;

	fstat(fd, &filestat);
	return filestat.st_mtim;
}

static void verify_copy_file_range_timestamp(void)
{
	loff_t offset;
	struct timespec timestamp1, timestamp2;
	long long diff_us;

	timestamp1 = get_timestamp(fd_dest);
	usleep(1500000);

	offset = 0;
	TEST(sys_copy_file_range(fd_src, &offset,
			fd_dest, 0, CONTSIZE, 0));
	if (TST_RET == -1)
		tst_brk(TBROK | TTERRNO,
				"copy_file_range unexpectedly failed");

	timestamp2 = get_timestamp(fd_dest);

	diff_us = tst_timespec_diff_us(timestamp2, timestamp1);

	if (diff_us >= 1000000 && diff_us <= 30000000)
		tst_res(TPASS, "copy_file_range sucessfully updated the timestamp");
	else
		tst_brk(TFAIL, "diff_us = %lld, copy_file_range might not update timestamp", diff_us);
}

static void cleanup(void)
{
	if (fd_dest > 0)
		SAFE_CLOSE(fd_dest);
	if (fd_src  > 0)
		SAFE_CLOSE(fd_src);
}

static void setup(void)
{
	syscall_info();

	fd_dest = SAFE_OPEN(FILE_DEST_PATH, O_RDWR | O_CREAT, 0664);
	fd_src  = SAFE_OPEN(FILE_SRC_PATH,  O_RDWR | O_CREAT, 0664);
	SAFE_WRITE(1, fd_src,  CONTENT,  CONTSIZE);
	SAFE_CLOSE(fd_src);
	fd_src = SAFE_OPEN(FILE_SRC_PATH, O_RDONLY);
}


static struct tst_test test = {
	.test_all = verify_copy_file_range_timestamp,
	.setup = setup,
	.cleanup = cleanup,
	.needs_tmpdir = 1,
	.test_variants = TEST_VARIANTS,
};
