// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) Linux Test Project, 2021
 * Copyright (c) International Business Machines  Corp., 2002
 * ported from SPIE, section2/filesuite/pread_pwrite.c, by Airong Zhang
 */

/*\
 * [Description]
 *
 * Test the pwrite() system call with O_APPEND.
 *
 * Writing 2k data to the file, close it and reopen it with O_APPEND.
 *
 * POSIX requires that opening a file with the O_APPEND flag should have no effect on the
 * location at which pwrite() writes data. However, on Linux, if a file is opened with
 * O_APPEND, pwrite() appends data to the end of the file, regardless of the value of offset.
 */

#include <stdlib.h>
#include <inttypes.h>
#include "tst_test.h"
#include "tst_safe_prw.h"

#define K1              1024
#define K2              (K1 * 2)
#define K3              (K1 * 3)
#define DATA_FILE       "pwrite04_file"

static int fd = -1;
static char *write_buf[2];

static void l_seek(int fdesc, off_t offset, int whence, off_t checkoff)
{
	off_t offloc;

	offloc = SAFE_LSEEK(fdesc, offset, whence);
	if (offloc != checkoff) {
		tst_res(TFAIL, "%" PRId64 " = lseek(%d, %" PRId64 ", %d) != %" PRId64,
			(int64_t)offloc, fdesc, (int64_t)offset, whence, (int64_t)checkoff);
	}
}

static void verify_pwrite(void)
{
	struct stat statbuf;

	fd = SAFE_OPEN(DATA_FILE, O_RDWR | O_CREAT | O_TRUNC, 0666);
	SAFE_PWRITE(1, fd, write_buf[0], K2, 0);
	SAFE_CLOSE(fd);

	fd = SAFE_OPEN(DATA_FILE, O_RDWR | O_APPEND, 0666);
	SAFE_FSTAT(fd, &statbuf);
	if (statbuf.st_size != K2)
		tst_res(TFAIL, "file size is %ld != K2", statbuf.st_size);

	/* Appends data to the end of the file regardless of offset. */
	l_seek(fd, K1, SEEK_SET, K1);
	SAFE_PWRITE(1, fd, write_buf[1], K1, 0);
	l_seek(fd, 0, SEEK_CUR, K1);
	SAFE_FSTAT(fd, &statbuf);
	if (statbuf.st_size != K3)
		tst_res(TFAIL, "file size is %ld != K3", statbuf.st_size);

	tst_res(TPASS, "O_APPEND test passed.");
	SAFE_CLOSE(fd);
}

static void setup(void)
{
	write_buf[0] = SAFE_MALLOC(K2);
	memset(write_buf[0], 0, K2);
	write_buf[1] = SAFE_MALLOC(K1);
	memset(write_buf[0], 1, K1);
}

static void cleanup(void)
{
	free(write_buf[0]);
	free(write_buf[1]);

	if (fd > -1)
		SAFE_CLOSE(fd);

	SAFE_UNLINK(DATA_FILE);
}

static struct tst_test test = {
	.needs_tmpdir = 1,
	.setup = setup,
	.cleanup = cleanup,
	.test_all = verify_pwrite,
};
