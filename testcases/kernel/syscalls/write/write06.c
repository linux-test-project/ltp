// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2021 FUJITSU LIMITED. All rights reserved.
 * Author: Dai Shili <daisl.fnst@fujitsu.com>
 */

/*\
 * Test the write() system call with O_APPEND.
 *
 * The full description of O_APPEND is in open(2) man-pages:
 * The file is opened in append mode.  Before each write(2), the
 * file offset is positioned at the end of the file, as if with lseek(2).
 * The modification of the file offset and the write operation are
 * performed as a single atomic step.
 *
 * Writing 2k data to the file, close it and reopen it with O_APPEND.
 * Verify that the file size is 3k and offset is moved to the end of the file.
 */

#include <stdlib.h>
#include <inttypes.h>
#include "tst_test.h"
#include "tst_safe_prw.h"

#define K1              1024
#define K2              (K1 * 2)
#define K3              (K1 * 3)
#define DATA_FILE       "write06_file"

static int fd = -1;
static char *write_buf[2];

static void verify_write(void)
{
	off_t off;
	struct stat statbuf;

	fd = SAFE_OPEN(DATA_FILE, O_RDWR | O_CREAT | O_TRUNC, 0666);
	SAFE_WRITE(SAFE_WRITE_ALL, fd, write_buf[0], K2);
	SAFE_CLOSE(fd);

	fd = SAFE_OPEN(DATA_FILE, O_RDWR | O_APPEND);
	SAFE_FSTAT(fd, &statbuf);
	if (statbuf.st_size != K2)
		tst_res(TFAIL, "file size is %ld != K2", statbuf.st_size);

	off = SAFE_LSEEK(fd, K1, SEEK_SET);
	if (off != K1)
		tst_brk(TBROK, "Failed to seek to K1");

	SAFE_WRITE(SAFE_WRITE_ALL, fd, write_buf[1], K1);

	off = SAFE_LSEEK(fd, 0, SEEK_CUR);
	if (off != K3)
		tst_res(TFAIL, "Wrong offset after write %zu expected %u", off, K3);
	else
		tst_res(TPASS, "Offset is correct after write %zu", off);

	SAFE_FSTAT(fd, &statbuf);
	if (statbuf.st_size != K3)
		tst_res(TFAIL, "Wrong file size after append %zu expected %u", statbuf.st_size, K3);
	else
		tst_res(TPASS, "Correct file size after append %u", K3);

	SAFE_CLOSE(fd);
}

static void setup(void)
{
	memset(write_buf[0], 0, K2);
	memset(write_buf[1], 1, K1);
}

static void cleanup(void)
{
	if (fd != -1)
		SAFE_CLOSE(fd);

	SAFE_UNLINK(DATA_FILE);
}

static struct tst_test test = {
	.needs_tmpdir = 1,
	.setup = setup,
	.cleanup = cleanup,
	.test_all = verify_write,
	.bufs = (struct tst_buffers[]) {
		{&write_buf[0], .size = K2},
		{&write_buf[1], .size = K1},
		{}
	}
};
