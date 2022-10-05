// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2017 Red Hat, Inc.  All rights reserved.
 * Author: Zorro Lang <zlang@redhat.com>
 *
 *  Test functional SEEK_HOLE and SEEK_DATA of lseek(2).
 *
 *  Since version 3.1, Linux supports the following additional values for
 *  whence:
 *
 *  SEEK_DATA
 *       Adjust the file offset to the next location in the file greater than
 *       or  equal  to  offset  containing data.  If offset points to data,
 *       then the file offset is set to offset.
 *
 *  SEEK_HOLE
 *       Adjust the file offset to the next hole in the file greater than or
 *       equal to offset.  If offset points into the middle of a hole, then
 *       the file offset is set to offset. If there is no hole past offset,
 *       then the file offset is adjusted to the end of the file (i.e., there
 *       is an implicit hole at the end of any file).
 */

#define _GNU_SOURCE
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "tst_test.h"
#include "tst_safe_prw.h"
#include "lapi/seek.h"

/*
 * This case create 3 holes and 4 data fields, every (data) is 12 bytes,
 * every UNIT has UNIT_BLOCKS * block_size bytes. The structure as below:
 *
 * ----------------------------------------------------------------------------------------------
 * data01suffix      (hole)      data02suffix      (hole)       data03suffix  (hole)  data04sufix
 * ----------------------------------------------------------------------------------------------
 * |<--- UNIT_BLOCKS blocks --->||<--- UNIT_BLOCKS blocks  --->||<---  UNIT_BLOCKS blocks   --->|
 *
 */
#define UNIT_COUNT   3
#define UNIT_BLOCKS  10
#define FILE_BLOCKS  (UNIT_BLOCKS * UNIT_COUNT)

static int fd;
static blksize_t block_size;

/*
 * SEEK from "startblock * block_size - offset", "whence" as the directive
 * whence.
 * startblock * block_size - offset: as offset of lseek()
 * whence: as whence of lseek()
 * data: as the expected result read from file offset. NULL means expect
 *       the end of file.
 * count: as the count read from file
 */
static struct tparam {
	off_t  startblock;
	off_t  offset;
	int    whence;
	char   *data;
	size_t count;
} tparams[] = {
	{0,               0,    SEEK_DATA, "data01",   6},    /* SEEK_DATA from starting of file*/
	{0,               4,    SEEK_DATA, "01suffix", 8},    /* SEEK_DATA from maddle of the first data */
	{0,               0,    SEEK_HOLE, "",         1023}, /* SEEK_HOLE from starting of file */
	{0,               4,    SEEK_HOLE, "",         1023}, /* SEEK_HOLE from maddle of the first data */
	{1,               0,    SEEK_HOLE, "",         1023}, /* SEEK_HOLE from the starting of the first hole */
	{1,               128,  SEEK_HOLE, "",         1023}, /* SEEK_HOLE from maddle of the first hole */
	{1,               0,    SEEK_DATA, "data02",   6},    /* SEEK_DATA from the starting of the first hole */
	{UNIT_BLOCKS,     -1,   SEEK_DATA, "data02",   6},    /* SEEK_DATA from the tail of the first hole */
	{UNIT_BLOCKS,     0,    SEEK_DATA, "data02",   6},    /* SEEK_DATA from the starting of the second data */
	{UNIT_BLOCKS,     4,    SEEK_DATA, "02suffix", 8},    /* SEEK_DATA from middle of the second data */
	{UNIT_BLOCKS,     0,    SEEK_HOLE, "",         1023}, /* SEEK_HOLE from the starting of the second data */
	{UNIT_BLOCKS,     4,    SEEK_HOLE, "",         1023}, /* SEEK_HOLE from middle of the second data */
	{UNIT_BLOCKS + 1, 128,  SEEK_HOLE, "",         1023}, /* SEEK_HOLE from middle of the second hole */
	{UNIT_BLOCKS + 1, 128,  SEEK_DATA, "data03",   6},    /* SEEK_DATA from middle of the second hole */
	{FILE_BLOCKS,    -128,  SEEK_HOLE, NULL,       0},    /* SEEK_HOLE from no hole pass offset*/
};

static void cleanup(void)
{
	SAFE_CLOSE(fd);
}

static void get_blocksize(void)
{
	off_t pos = 0, offset = 128;
	int shift;
	struct stat st;

	SAFE_FSTAT(fd, &st);

	/* try to discover the actual alloc size */
	while (pos == 0 && offset < (st.st_blksize * 2)) {
		offset <<= 1;
		SAFE_FTRUNCATE(fd, 0);
		SAFE_PWRITE(1, fd, "a", 1, offset);
		SAFE_FSYNC(fd);
		pos = lseek(fd, 0, SEEK_DATA);
		if (pos == -1) {
			if (errno == EINVAL || errno == EOPNOTSUPP) {
				tst_brk(TCONF | TERRNO, "SEEK_DATA "
					"and SEEK_HOLE not implemented");
			}
			tst_brk(TBROK | TERRNO, "SEEK_DATA failed");
		}
	}

	/* bisect for double check */
	shift = offset >> 2;
	while (shift && offset < (st.st_blksize * 2)) {
		SAFE_FTRUNCATE(fd, 0);
		SAFE_PWRITE(1, fd, "a", 1, offset);
		SAFE_FSYNC(fd);
		pos = SAFE_LSEEK(fd, 0, SEEK_DATA);
		offset += pos ? -shift : shift;
		shift >>= 1;
	}

	if (!shift)
		offset += pos ? 0 : 1;
	block_size = offset;

	/*
	 * Due to some filesystems use generic_file_llseek(), e.g: CIFS,
	 * it thinks the entire file is data, only a virtual hole at the end
	 * of the file. This case can't test this situation, so if the minimum
	 * alloc size we got bigger then st.st_blksize, we think it's not
	 * a valid value.
	 */
	if (block_size > st.st_blksize) {
		tst_brk(TCONF,
		        "filesystem maybe use generic_file_llseek(), not support real SEEK_DATA/SEEK_HOLE");
	}
}

static void write_data(int fd, int num)
{
	char buf[64];

	sprintf(buf, "data%02dsuffix", num);
	SAFE_WRITE(SAFE_WRITE_ALL, fd, buf, strlen(buf));
}

static void setup(void)
{
	int i;
	off_t offset = 0;
	char fname[255];

	sprintf(fname, "tfile_lseek_%d", getpid());

	fd = SAFE_OPEN(fname, O_RDWR | O_CREAT, 0666);

	get_blocksize();
	tst_res(TINFO, "The block size is %lu", block_size);

	/*
	 * truncate to the expected file size directly, to keep away the effect
	 * of speculative preallocation of some filesystems (e.g. XFS)
	 */
	SAFE_FTRUNCATE(fd, FILE_BLOCKS * block_size);

	SAFE_LSEEK(fd, 0, SEEK_HOLE);

	for (i = 0; i < UNIT_COUNT; i++) {
		offset = UNIT_BLOCKS * block_size * i;
		SAFE_LSEEK(fd, offset, SEEK_SET);
		write_data(fd, i + 1);
	}

	SAFE_LSEEK(fd, -128, SEEK_END);
	write_data(fd, i + 1);

	SAFE_FSYNC(fd);
	SAFE_LSEEK(fd, 0, SEEK_SET);
}

static void test_lseek(unsigned int n)
{
	struct tparam *tp = &tparams[n];
	off_t offset;
	char buf[1024];
	int rc = 0;

	memset(buf, 0, sizeof(buf));
	offset = (tp->startblock * block_size) + tp->offset;
	offset = SAFE_LSEEK(fd, offset, tp->whence);
	if (tp->data) {
		SAFE_READ(1, fd, buf, tp->count);
		rc = strcmp(buf, tp->data);
	} else {
		if (offset != SAFE_LSEEK(fd, 0, SEEK_END))
			rc = 1;
	}

	if (rc != 0) {
		tst_res(TFAIL,
		        "The %uth test failed: %s from startblock %ld offset %ld, expect \'%s\' return \'%s\'",
		        n, (tp->whence == SEEK_DATA) ? "SEEK_DATA" : "SEEK_HOLE",
		        tp->startblock, tp->offset, tp->data ? tp->data : "", buf);
	} else {
		tst_res(TPASS,
		        "The %uth test passed: %s from startblock %ld offset %ld",
		        n, (tp->whence == SEEK_DATA) ? "SEEK_DATA" : "SEEK_HOLE",
		        tp->startblock, tp->offset);
	}
}

static struct tst_test test = {
	.tcnt         = ARRAY_SIZE(tparams),
	.test         = test_lseek,
	.setup        = setup,
	.cleanup      = cleanup,
	.needs_tmpdir = 1,
};
