// SPDX-License-Identifier: GPL-2.0-or-later

/*
 * Copyright (c) International Business Machines  Corp., 2007
 * Author: Sharyathi Nagesh <sharyathi@in.ibm.com>
 * Copyright (c) Linux Test Project, 2008-2017
 * Copyright (C) 2024 SUSE LLC Andrea Manzini <andrea.manzini@suse.com>
 */

/*\
 * [Description]
 * Test fallocate() on sparse file for different offsets, with a total of 8 test cases
 */

#define _GNU_SOURCE

#include <fcntl.h>
#include "tst_test.h"

#define BLOCKS_WRITTEN 12
#define HOLE_SIZE_IN_BLOCKS 12
#define DEFAULT_MODE 0

static int fd;
static struct test_case {
	int mode;
	loff_t offset;
} test_cases[] = {
	{DEFAULT_MODE, 2},
	{DEFAULT_MODE, BLOCKS_WRITTEN},
	{DEFAULT_MODE, BLOCKS_WRITTEN + HOLE_SIZE_IN_BLOCKS / 2 - 1},
	{DEFAULT_MODE, BLOCKS_WRITTEN + HOLE_SIZE_IN_BLOCKS + 1},
	{FALLOC_FL_KEEP_SIZE, 2},
	{FALLOC_FL_KEEP_SIZE, BLOCKS_WRITTEN},
	{FALLOC_FL_KEEP_SIZE, BLOCKS_WRITTEN + HOLE_SIZE_IN_BLOCKS / 2 + 1},
	{FALLOC_FL_KEEP_SIZE, BLOCKS_WRITTEN + HOLE_SIZE_IN_BLOCKS + 2}
};

static int block_size;

static void cleanup(void)
{
	if (fd)
		SAFE_CLOSE(fd);
}

static void write_data_to_file(int buf_size)
{
	char buf[buf_size + 1];

	for (int blocks = 0; blocks < BLOCKS_WRITTEN; blocks++) {
		for (int i = 0; i < buf_size; i++)
			buf[i] = 'A' + (i % 26);

		buf[buf_size] = '\0';
		SAFE_WRITE(SAFE_WRITE_ALL, fd, buf, buf_size);
	}
}

static void setup(void)
{
	char fname[NAME_MAX];
	struct stat file_stat;

	sprintf(fname, "tfile_sparse_%d", getpid());
	fd = SAFE_OPEN(fname, O_RDWR | O_CREAT, 0700);

	SAFE_FSTAT(fd, &file_stat);
	block_size = (int)file_stat.st_blksize;

	write_data_to_file(block_size);
	SAFE_LSEEK(fd, block_size * (BLOCKS_WRITTEN + HOLE_SIZE_IN_BLOCKS), SEEK_SET);
	write_data_to_file(block_size);
	SAFE_LSEEK(fd, 0, SEEK_SET);
}


static void verify_fallocate(unsigned int nr)
{
	struct test_case *tc = &test_cases[nr];

	TST_EXP_PASS(
		fallocate(fd, tc->mode, tc->offset * block_size, block_size),
		"fallocate(fd, %s, %ld, %d)",
		tc->mode ? "FALLOC_FL_KEEP_SIZE" : "DEFAULT_MODE",
		tc->offset * block_size, block_size);
}

static struct tst_test test = {
	.setup = setup,
	.cleanup = cleanup,
	.test = verify_fallocate,
	.needs_tmpdir = 1,
	.tcnt = ARRAY_SIZE(test_cases)
};
