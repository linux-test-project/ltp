// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines  Corp., 2001
 *   07/2001 Ported by Wayne Boyer
 * Copyright (c) 2013 Cyril Hrubis <chrubis@suse.cz>
 */

/*
 * DESCRIPTION
 *	Testcase to check the basic functionality of the readv(2) system call.
 *
 * ALGORITHM
 *	Create a IO vector, and attempt to readv() various components of it.
 */
#include <stdlib.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <fcntl.h>
#include <memory.h>

#include "tst_test.h"

/* Note: multi_iovec test assumes CHUNK is divisible by 4 */
#define	CHUNK		64

static char buf[CHUNK];
static struct iovec *rd_iovec, *big_iovec, *multi_iovec, *lockup_iovec;
static int fd;

static struct testcase {
	struct iovec **iov;
	int iov_count, exp_ret;
	const char *name;
} testcase_list[] = {
	{&rd_iovec, 0, 0, "readv() with 0 I/O vectors"},
	{&rd_iovec, 3, CHUNK, "readv() with NULL I/O vectors"},
	{&big_iovec, 2, CHUNK, "readv() with too big I/O vectors"},
	{&multi_iovec, 2, 3*CHUNK/4, "readv() with multiple I/O vectors"},
	{&lockup_iovec, 2, CHUNK, "readv() with zero-len buffer"},
};

static void test_readv(unsigned int n)
{
	int i, fpos, fail = 0;
	size_t j;
	char *ptr;
	const struct testcase *tc = testcase_list + n;
	struct iovec *vec;

	SAFE_LSEEK(fd, 0, SEEK_SET);
	vec = *tc->iov;

	for (i = 0; i < tc->iov_count; i++) {
		if (vec[i].iov_base && vec[i].iov_len)
			memset(vec[i].iov_base, 0, vec[i].iov_len);
	}

	TEST(readv(fd, vec, tc->iov_count));

	if (TST_RET == -1)
		tst_res(TFAIL | TTERRNO, "readv() failed unexpectedly");
	else if (TST_RET < 0)
		tst_res(TFAIL | TTERRNO, "readv() returned invalid value");
	else if (TST_RET != tc->exp_ret)
		tst_res(TFAIL, "readv() returned unexpected value %ld",
			TST_RET);

	if (TST_RET != tc->exp_ret)
		return;

	tst_res(TPASS, "%s", tc->name);

	for (i = 0, fpos = 0; i < tc->iov_count; i++) {
		ptr = vec[i].iov_base;

		for (j = 0; j < vec[i].iov_len; j++, fpos++) {
			if (ptr[j] != (fpos < tc->exp_ret ? 0x42 : 0))
				fail++;
		}
	}

	if (fail)
		tst_res(TFAIL, "Wrong buffer content");
	else
		tst_res(TPASS, "readv() correctly read %d bytes ", tc->exp_ret);
}

static void setup(void)
{
	/* replace the default NULL pointer with invalid address */
	lockup_iovec[0].iov_base = tst_get_bad_addr(NULL);

	memset(buf, 0x42, sizeof(buf));

	fd = SAFE_OPEN("data_file", O_WRONLY | O_CREAT | O_TRUNC, 0666);
	SAFE_WRITE(SAFE_WRITE_ALL, fd, buf, sizeof(buf));
	SAFE_CLOSE(fd);
	fd = SAFE_OPEN("data_file", O_RDONLY);
}

static void cleanup(void)
{
	if (fd >= 0)
		SAFE_CLOSE(fd);
}

static struct tst_test test = {
	.setup = setup,
	.cleanup = cleanup,
	.test = test_readv,
	.tcnt = ARRAY_SIZE(testcase_list),
	.needs_tmpdir = 1,
	.tags = (const struct tst_tag[]) {
		{"linux-git", "19f18459330f"},
		{}
	},
	.bufs = (struct tst_buffers[]) {
		{&rd_iovec, .iov_sizes = (int[]){CHUNK, 0, 0, -1}},
		{&big_iovec, .iov_sizes = (int[]){2*CHUNK, CHUNK, -1}},
		{&multi_iovec, .iov_sizes = (int[]){CHUNK/4, CHUNK/2, -1}},
		{&lockup_iovec, .iov_sizes = (int[]){0, CHUNK, -1}},
		{}
	}
};
