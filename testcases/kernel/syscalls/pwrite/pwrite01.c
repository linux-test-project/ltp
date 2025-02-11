// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) Linux Test Project, 2021
 * Copyright (c) International Business Machines  Corp., 2001
 * 07/2001 Ported by Wayne Boyer
 */

/*\
 * Verify the functionality of pwrite() by writing known data using pwrite()
 * to the file at various specified offsets and later read from the file from
 * various specified offsets, comparing the data written aganist the data
 * read using read().
 */

#include <stdlib.h>
#include <inttypes.h>
#include "tst_test.h"
#include "tst_safe_prw.h"

#define TEMPFILE	"pwrite_file"
#define K1              1024
#define K2              (K1 * 2)
#define K3              (K1 * 3)
#define K4              (K1 * 4)
#define NBUFS           4

static int fildes;
static char *write_buf[NBUFS];
static char *read_buf[NBUFS];

static void l_seek(int fdesc, off_t offset, int whence, off_t checkoff)
{
	off_t offloc;

	offloc = SAFE_LSEEK(fdesc, offset, whence);
	if (offloc != checkoff) {
		tst_res(TFAIL, "return = %" PRId64 ", expected %" PRId64,
			(int64_t) offloc, (int64_t) checkoff);
	}
}

static void check_file_contents(void)
{
	int count, err_flg = 0;

	for (count = 0; count < NBUFS; count++) {
		l_seek(fildes, count * K1, SEEK_SET, count * K1);

		SAFE_READ(1, fildes, read_buf[count], K1);

		if (memcmp(write_buf[count], read_buf[count], K1) != 0) {
			tst_res(TFAIL, "read/write buffer[%d] data mismatch", count);
			err_flg++;
		}
	}

	if (!err_flg)
		tst_res(TPASS, "Functionality of pwrite() successful");
}

static void verify_pwrite(void)
{
	SAFE_PWRITE(1, fildes, write_buf[0], K1, 0);
	l_seek(fildes, 0, SEEK_CUR, 0);
	l_seek(fildes, K1 / 2, SEEK_SET, K1 / 2);

	SAFE_PWRITE(1, fildes, write_buf[2], K1, K2);
	l_seek(fildes, 0, SEEK_CUR, K1 / 2);
	l_seek(fildes, K3, SEEK_SET, K3);

	SAFE_WRITE(SAFE_WRITE_ALL, fildes, write_buf[3], K1);
	l_seek(fildes, 0, SEEK_CUR, K4);

	SAFE_PWRITE(1, fildes, write_buf[1], K1, K1);

	check_file_contents();

	l_seek(fildes, 0, SEEK_SET, 0);
}

static void setup(void)
{
	int count;

	for (count = 0; count < NBUFS; count++) {
		write_buf[count] = SAFE_MALLOC(K1);
		read_buf[count] = SAFE_MALLOC(K1);
		memset(write_buf[count], count, K1);
	}

	fildes = SAFE_OPEN(TEMPFILE, O_RDWR | O_CREAT, 0666);
}

static void cleanup(void)
{
	int count;

	for (count = 0; count < NBUFS; count++) {
		free(write_buf[count]);
		free(read_buf[count]);
	}

	if (fildes > 0)
		SAFE_CLOSE(fildes);
}

static struct tst_test test = {
	.needs_tmpdir = 1,
	.setup = setup,
	.cleanup = cleanup,
	.test_all = verify_pwrite,
};
