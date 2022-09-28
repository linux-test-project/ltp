// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) Zilogic Systems Pvt. Ltd., 2018
 * Email : code@zilogic.com
 */

/*\
 * [Description]
 *
 * Test the following file timestamps of statx syscall:
 *
 * - btime - The time before and after the execution of the create system call is noted.
 *
 * - mtime - The time before and after the execution of the write system call is noted.
 *
 * - atime - The time before and after the execution of the read system call is noted.
 *
 * - ctime - The time before and after the execution of the chmod system call is noted.
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <time.h>

#include "tst_test.h"
#include "tst_safe_clocks.h"
#include "tst_safe_macros.h"
#include "tst_timer.h"
#include "lapi/stat.h"
#include "lapi/mount.h"
#include "lapi/fcntl.h"

#define MOUNT_POINT "mount_ext"
#define TEST_FILE MOUNT_POINT"/test_file.txt"
#define SIZE 2

static int fd;

static void timestamp_to_timespec(const struct statx_timestamp *timestamp,
				  struct timespec *timespec)
{
	timespec->tv_sec = timestamp->tv_sec;
	timespec->tv_nsec = timestamp->tv_nsec;
}

static void clock_wait_tick(void)
{
	struct timespec res;
	unsigned int usecs;

	SAFE_CLOCK_GETRES(CLOCK_REALTIME_COARSE, &res);
	usecs = tst_timespec_to_us(res);

	usleep(usecs);
}

static void create_file(void)
{
	if (fd > 0) {
		SAFE_CLOSE(fd);
		SAFE_UNLINK(TEST_FILE);
	}
	fd = SAFE_OPEN(TEST_FILE, O_CREAT | O_RDWR, 0666);
}

static void write_file(void)
{
	char data[SIZE] = "hi";

	SAFE_WRITE(0, fd, data, sizeof(data));
}

static void read_file(void)
{
	char data[SIZE];

	SAFE_READ(0, fd, data, sizeof(data));
}

static void change_mode(void)
{
	SAFE_CHMOD(TEST_FILE, 0777);
}

static struct test_case {
	void (*operation)(void);
	char *op_name;
} tcases[] = {
	{.operation = create_file,
	 .op_name = "Birth time"},
	{.operation = write_file,
	 .op_name = "Modified time"},
	{.operation = read_file,
	 .op_name = "Access time"},
	{.operation = change_mode,
	 .op_name = "Change time"}
};

static void test_statx(unsigned int test_nr)
{
	struct statx buff;
	struct timespec before_time;
	struct timespec after_time;
	struct timespec statx_time = {0, 0};

	struct test_case *tc = &tcases[test_nr];

	SAFE_CLOCK_GETTIME(CLOCK_REALTIME_COARSE, &before_time);
	clock_wait_tick();
	tc->operation();
	clock_wait_tick();
	SAFE_CLOCK_GETTIME(CLOCK_REALTIME_COARSE, &after_time);

	TEST(statx(AT_FDCWD, TEST_FILE, 0, STATX_ALL, &buff));
	if (TST_RET != 0) {
		tst_brk(TFAIL | TTERRNO,
			"statx(AT_FDCWD, %s, 0, STATX_ALL, &buff)",
			TEST_FILE);
	}

	switch (test_nr) {
	case 0:
		timestamp_to_timespec(&buff.stx_btime, &statx_time);
		break;
	case 1:
		timestamp_to_timespec(&buff.stx_mtime, &statx_time);
		break;
	case 2:
		timestamp_to_timespec(&buff.stx_atime, &statx_time);
		break;
	case 3:
		timestamp_to_timespec(&buff.stx_ctime, &statx_time);
		break;
	}
	if (tst_timespec_lt(statx_time, before_time))
		tst_res(TFAIL, "%s < before time", tc->op_name);
	else if (tst_timespec_lt(after_time, statx_time))
		tst_res(TFAIL, "%s > after_time", tc->op_name);
	else
		tst_res(TPASS, "%s Passed", tc->op_name);
}


static void cleanup(void)
{
	if (fd > 0)
		SAFE_CLOSE(fd);
}

static struct tst_test test = {
	.cleanup = cleanup,
	.tcnt = ARRAY_SIZE(tcases),
	.test = test_statx,
	.min_kver = "4.11",
	.needs_root = 1,
	.mntpoint = MOUNT_POINT,
	.mount_device = 1,
	.dev_fs_type = "ext4",
	.dev_fs_opts = (const char *const []){"-I", "256", NULL},
	.mnt_flags = MS_STRICTATIME,
};
