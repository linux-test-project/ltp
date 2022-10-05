// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) Zilogic Systems Pvt. Ltd., 2020
 * Email: code@zilogic.com
 */

/*
 * Test mmap with MAP_FIXED_NOREPLACE flag
 *
 * We are testing the MAP_FIXED_NOREPLACE flag of mmap() syscall. To check
 * if an attempt to mmap at an exisiting mapping fails with EEXIST.
 * The code allocates a free address by passing NULL to first mmap call
 * Then tries to mmap with the same address using MAP_FIXED_NOREPLACE flag
 * and the mapping fails as expected.
 */

#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include "lapi/mmap.h"
#include "tst_test.h"

static int fd_file1;
static int fd_file2;
static void *mapped_address;
static const char str[] = "Writing to mapped file";

#define FNAME1 "file1_to_mmap"
#define FNAME2 "file2_to_mmap"

static void setup(void)
{
	fd_file1 = SAFE_OPEN(FNAME1, O_CREAT | O_RDWR, 0600);
	fd_file2 = SAFE_OPEN(FNAME2, O_CREAT | O_RDWR, 0600);
}

static void cleanup(void)
{
	int str_len;

	str_len = strlen(str);

	if (fd_file2 > 0)
		SAFE_CLOSE(fd_file2);
	if (fd_file1 > 0)
		SAFE_CLOSE(fd_file1);
	if (mapped_address)
		SAFE_MUNMAP(mapped_address, str_len);
}

static void test_mmap(void)
{
	int str_len;
	void *address;

	str_len = strlen(str);

	SAFE_WRITE(SAFE_WRITE_ALL, fd_file1, str, str_len);
	mapped_address = SAFE_MMAP(NULL, str_len, PROT_WRITE,
				   MAP_PRIVATE, fd_file1, 0);

	SAFE_WRITE(SAFE_WRITE_ALL, fd_file2, str, str_len);

	address = mmap(mapped_address, str_len, PROT_WRITE,
		  MAP_PRIVATE | MAP_FIXED_NOREPLACE, fd_file2, 0);
	if (address == MAP_FAILED && errno == EEXIST)
		tst_res(TPASS, "mmap set errno to EEXIST as expected");
	else
		tst_res(TFAIL | TERRNO, "mmap failed, with unexpected error "
			"code, expected EEXIST");
}

static struct tst_test test = {
	.setup = setup,
	.cleanup = cleanup,
	.test_all = test_mmap,
	.min_kver = "4.17",
	.needs_tmpdir = 1
};
