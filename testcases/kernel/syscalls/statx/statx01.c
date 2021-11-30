// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) Zilogic Systems Pvt. Ltd., 2018
 * Email: code@zilogic.com
 */

/*
 * Test statx
 *
 * This code tests the functionality of statx system call.
 *
 * TESTCASE 1:
 * The metadata for normal file are tested against predefined values:
 * 1) gid
 * 2) uid
 * 3) mode
 * 4) blocks
 * 5) size
 * 6) nlink
 *
 * A file is created and metadata values are set with
 * predefined values.
 * Then the values obtained using statx is checked against
 * the predefined values.
 *
 * TESTCASE 2:
 * The metadata for device file are tested against predefined values:
 * 1) MAJOR number
 * 2) MINOR number
 *
 * A device file is created seperately using mknod(must be a root user).
 * The major number and minor number are set while creation.
 * Major and minor numbers obtained using statx is checked against
 * predefined values.
 * Minimum kernel version required is 4.11.
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <sys/types.h>
#include <sys/sysmacros.h>
#include "tst_test.h"
#include "tst_safe_macros.h"
#include "lapi/stat.h"
#include <string.h>
#include <inttypes.h>

#define TESTFILE "test_file"
#define MNTPOINT "mntpoint/"
#define DEVICEFILE MNTPOINT"blk_dev"
#define MODE 0644

#define SIZE 256
#define MAJOR 8
#define MINOR 1

static void test_normal_file(void)
{
	struct statx buff;

	TEST(statx(AT_FDCWD, TESTFILE, 0, 0, &buff));
	if (TST_RET == 0)
		tst_res(TPASS,
			"statx(AT_FDCWD, %s, 0, 0, &buff)", TESTFILE);
	else
		tst_brk(TFAIL | TTERRNO,
			"statx(AT_FDCWD, %s, 0, 0, &buff)", TESTFILE);

	if (geteuid() == buff.stx_uid)
		tst_res(TPASS, "stx_uid(%u) is correct", buff.stx_uid);
	else
		tst_res(TFAIL, "stx_uid(%u) is different from euid(%u)",
			buff.stx_uid, geteuid());

	if (getegid() == buff.stx_gid)
		tst_res(TPASS, "stx_gid(%u) is correct", buff.stx_gid);
	else
		tst_res(TFAIL, "stx_gid(%u) is different from egid(%u)",
			buff.stx_gid, getegid());

	if (buff.stx_size == SIZE)
		tst_res(TPASS,
			"stx_size(%"PRIu64") is correct", (uint64_t)buff.stx_size);
	else
		tst_res(TFAIL,
			"stx_size(%"PRIu64") is different from expected(%u)",
			(uint64_t)buff.stx_size, SIZE);

	if ((buff.stx_mode & ~(S_IFMT)) == MODE)
		tst_res(TPASS, "stx_mode(%u) is correct", buff.stx_mode);
	else
		tst_res(TFAIL, "stx_mode(%u) is different from expected(%u)",
			buff.stx_mode, MODE);


	if (buff.stx_blocks <= buff.stx_blksize/512 * 2)
		tst_res(TPASS, "stx_blocks(%"PRIu64") is valid",
			(uint64_t)buff.stx_blocks);
	else
		tst_res(TFAIL, "stx_blocks(%"PRIu64") is invalid",
			(uint64_t)buff.stx_blocks);

	if (buff.stx_nlink == 1)
		tst_res(TPASS, "stx_nlink(1) is correct");
	else
		tst_res(TFAIL, "stx_nlink(%u) is different from expected(1)",
			buff.stx_nlink);

}

static void test_device_file(void)
{
	struct statx buff;

	TEST(statx(AT_FDCWD, DEVICEFILE, 0, 0, &buff));
	if (TST_RET == 0)
		tst_res(TPASS,
			"statx(AT_FDCWD, %s, 0, 0, &buff)", DEVICEFILE);
	else
		tst_brk(TFAIL | TTERRNO,
			"statx(AT_FDCWD, %s, 0, 0, &buff)", DEVICEFILE);

	if (buff.stx_rdev_major == MAJOR)
		tst_res(TPASS, "stx_rdev_major(%u) is correct",
			buff.stx_rdev_major);
	else
		tst_res(TFAIL,
			"stx_rdev_major(%u) is different from expected(%u)",
			buff.stx_rdev_major, MAJOR);

	if (buff.stx_rdev_minor == MINOR)
		tst_res(TPASS, "stx_rdev_minor(%u) is correct",
			buff.stx_rdev_minor);
	else
		tst_res(TFAIL,
			"stx_rdev_minor(%u) is different from expected(%u)",
			buff.stx_rdev_minor, MINOR);
}


struct tcase {
	void (*tfunc)(void);
} tcases[] = {
	{&test_normal_file},
	{&test_device_file}
};

static void run(unsigned int i)
{
	tcases[i].tfunc();
}

static void setup(void)
{
	char data_buff[SIZE];
	int file_fd;

	umask(0);

	memset(data_buff, '@', sizeof(data_buff));

	file_fd =  SAFE_OPEN(TESTFILE, O_RDWR|O_CREAT, MODE);
	SAFE_WRITE(1, file_fd, data_buff, sizeof(data_buff));
	SAFE_CLOSE(file_fd);

	SAFE_MKNOD(DEVICEFILE, S_IFBLK | 0777, makedev(MAJOR, MINOR));
}

static struct tst_test test = {
	.test = run,
	.tcnt = ARRAY_SIZE(tcases),
	.setup = setup,
	.min_kver = "4.11",
	.needs_devfs = 1,
	.mntpoint = MNTPOINT,
	.needs_root = 1,
};
