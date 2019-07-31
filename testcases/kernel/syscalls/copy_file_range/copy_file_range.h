// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2019 SUSE LLC
 * Author: Christian Amann <camann@suse.com>
 */

#ifndef __COPY_FILE_RANGE_H__
#define __COPY_FILE_RANGE_H__

#include <stdbool.h>
#include <unistd.h>
#include <sys/sysmacros.h>
#include <limits.h>
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
#define MAX_LEN   MAX_LFS_FILESIZE
#define MIN_OFF   65537
#define MAX_OFF   (MAX_LEN - MIN_OFF)

static void syscall_info(void)
{
	switch (tst_variant) {
	case 0:
		tst_res(TINFO, "Testing libc copy_file_range()");
		break;
	case 1:
		tst_res(TINFO, "Testing tst copy_file_range()");
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

#endif /* __COPY_FILE_RANGE_H__ */
