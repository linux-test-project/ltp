// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) Zilogic Systems Pvt. Ltd., 2020
 *	Email: code@zilogic.com
 * Copyright (C) 2025 SUSE LLC Andrea Cervesato <andrea.cervesato@suse.com>
 */

/*\
 * Verify MAP_FIXED_NOREPLACE flag for the mmap() syscall and check
 * if an attempt to mmap at an exisiting mapping fails with EEXIST.
 */

#include "tst_test.h"
#include "lapi/mmap.h"

static int fd_file1;
static int fd_file2;
static void *mapped_address;
static const char msg[] = "Writing to mapped file";
static int msg_len;

#define FNAME1 "file1_to_mmap"
#define FNAME2 "file2_to_mmap"

static void setup(void)
{
	msg_len = strlen(msg);

	fd_file1 = SAFE_OPEN(FNAME1, O_CREAT | O_RDWR, 0600);
	fd_file2 = SAFE_OPEN(FNAME2, O_CREAT | O_RDWR, 0600);

	SAFE_WRITE(SAFE_WRITE_ALL, fd_file1, msg, msg_len);
	SAFE_WRITE(SAFE_WRITE_ALL, fd_file2, msg, msg_len);

	mapped_address = SAFE_MMAP(NULL, msg_len,
		PROT_WRITE, MAP_PRIVATE, fd_file1, 0);
}

static void cleanup(void)
{
	if (fd_file2 > 0)
		SAFE_CLOSE(fd_file2);
	if (fd_file1 > 0)
		SAFE_CLOSE(fd_file1);
	if (mapped_address)
		SAFE_MUNMAP(mapped_address, msg_len);
}

static void test_mmap(void)
{
	void *address;

	address = mmap(mapped_address, msg_len, PROT_WRITE,
		  MAP_PRIVATE | MAP_FIXED_NOREPLACE, fd_file2, 0);

	if (address == MAP_FAILED && errno == EEXIST)
		tst_res(TPASS, "mmap set errno to EEXIST as expected");
	else
		tst_res(TFAIL | TERRNO, "mmap failed, with unexpected error "
			"code, expected EEXIST");

	if (address != MAP_FAILED)
		SAFE_MUNMAP(address, msg_len);
}

static struct tst_test test = {
	.setup = setup,
	.cleanup = cleanup,
	.test_all = test_mmap,
	.min_kver = "4.17",
	.needs_tmpdir = 1
};
