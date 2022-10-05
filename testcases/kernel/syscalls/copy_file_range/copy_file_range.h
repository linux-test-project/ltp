// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2019 SUSE LLC
 * Author: Christian Amann <camann@suse.com>
 */

#ifndef __COPY_FILE_RANGE_H__
#define __COPY_FILE_RANGE_H__

#include <stdio.h>
#include "lapi/syscalls.h"
#include "lapi/fs.h"

#define TEST_VARIANTS	2

#define MNTPOINT	"mnt_point"
#define FILE_SRC_PATH   "file_src"
#define FILE_DEST_PATH  "file_dest"
#define FILE_RDONL_PATH "file_rdonl"
#define FILE_DIR_PATH	"file_dir"
#define FILE_MNTED_PATH  MNTPOINT"/file_mnted"
#define FILE_IMMUTABLE_PATH "file_immutable"
#define FILE_SWAP_PATH "file_swap"
#define FILE_CHRDEV    "/dev/null"
#define FILE_FIFO      "file_fifo"
#define FILE_COPY_PATH  "file_copy"

#define CONTENT		"ABCDEFGHIJKLMNOPQRSTUVWXYZ12345\n"
#define CONTSIZE	(sizeof(CONTENT) - 1)
#define MIN_OFF   65537

static void syscall_info(void)
{
	switch (tst_variant) {
	case 0:
		tst_res(TINFO, "Testing libc copy_file_range()");
		break;
	case 1:
		tst_res(TINFO, "Testing __NR_copy_file_range syscall");
	}
}

static int sys_copy_file_range(int fd_in, loff_t *off_in,
		int fd_out, loff_t *off_out, size_t len, unsigned int flags)
{
	switch (tst_variant) {

	case 0:
#ifdef HAVE_COPY_FILE_RANGE
		return copy_file_range(fd_in, off_in,
				fd_out, off_out, len, flags);
#else
		tst_brk(TCONF, "libc copy_file_range() not supported");
#endif
		break;
	case 1:
		return tst_syscall(__NR_copy_file_range, fd_in, off_in, fd_out,
				off_out, len, flags);
	}
	return -1;
}

static inline int verify_cross_fs_copy_support(const char *path_in, const char *path_out)
{
	int i, fd, fd_test;

	fd = SAFE_OPEN(path_in, O_RDWR | O_CREAT, 0664);
	/* Writing page_size * 4 of data into test file */
	for (i = 0; i < (int)(getpagesize() * 4); i++)
		SAFE_WRITE(SAFE_WRITE_ALL, fd, CONTENT, CONTSIZE);

	fd_test = SAFE_OPEN(path_out, O_RDWR | O_CREAT, 0664);
	TEST(sys_copy_file_range(fd, 0, fd_test, 0, CONTSIZE, 0));

	SAFE_CLOSE(fd_test);
	remove(FILE_MNTED_PATH);
	SAFE_CLOSE(fd);

	return TST_ERR == EXDEV ? 0 : 1;
}

#endif /* __COPY_FILE_RANGE_H__ */
