// SPDX-License-Identifier: GPL-2.0-or-later
/*
 *
 *   Copyright (c) Linux Test Project, 2016
 */

/*
 * Test Description:
 *   Verify writev() behaviour with partially valid iovec list.
 *   Kernel <4.8 used to shorten write up to first bad invalid
 *   iovec. Starting with 4.8, a writev with short data (under
 *   page size) is likely to get shorten to 0 bytes and return
 *   EFAULT.
 *
 *   This test doesn't make assumptions how much will write get
 *   shortened. It only tests that file content/offset after
 *   syscall corresponds to return value of writev().
 *
 *   See: [RFC] writev() semantics with invalid iovec in the middle
 *        https://marc.info/?l=linux-kernel&m=147388891614289&w=2
 */

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/uio.h>
#include "tst_test.h"

#define TESTFILE "testfile"
#define CHUNK 64
#define BUFSIZE (CHUNK * 4)

static void *bad_addr;

static void test_partially_valid_iovec(int initial_file_offset)
{
	int i, fd;
	unsigned char buffer[BUFSIZE], fpattern[BUFSIZE], tmp[BUFSIZE];
	long off_after;
	struct iovec wr_iovec[] = {
		{ buffer, CHUNK },
		{ bad_addr, CHUNK },
		{ buffer + CHUNK, CHUNK },
		{ buffer + CHUNK * 2, CHUNK },
	};

	tst_res(TINFO, "starting test with initial file offset: %d ",
		initial_file_offset);

	for (i = 0; i < BUFSIZE; i++)
		buffer[i] = i % (CHUNK - 1);

	memset(fpattern, 0xff, BUFSIZE);
	tst_fill_file(TESTFILE, 0xff, CHUNK, BUFSIZE / CHUNK);

	fd = SAFE_OPEN(TESTFILE, O_RDWR, 0644);
	SAFE_LSEEK(fd, initial_file_offset, SEEK_SET);
	TEST(writev(fd, wr_iovec, ARRAY_SIZE(wr_iovec)));
	off_after = (long) SAFE_LSEEK(fd, 0, SEEK_CUR);

	/* bad errno */
	if (TST_RET == -1 && TST_ERR != EFAULT) {
		tst_res(TFAIL | TTERRNO, "unexpected errno");
		SAFE_CLOSE(fd);
		return;
	}

	/* nothing has been written */
	if (TST_RET == -1 && TST_ERR == EFAULT) {
		tst_res(TINFO, "got EFAULT");
		/* initial file content remains untouched */
		SAFE_LSEEK(fd, 0, SEEK_SET);
		SAFE_READ(1, fd, tmp, BUFSIZE);
		if (memcmp(tmp, fpattern, BUFSIZE))
			tst_res(TFAIL, "file was written to");
		else
			tst_res(TPASS, "file stayed untouched");

		/* offset hasn't changed */
		if (off_after == initial_file_offset)
			tst_res(TPASS, "offset stayed unchanged");
		else
			tst_res(TFAIL, "offset changed to %ld",
				off_after);

		SAFE_CLOSE(fd);
		return;
	}

	/* writev() wrote more bytes than bytes preceding invalid iovec */
	tst_res(TINFO, "writev() has written %ld bytes", TST_RET);
	if (TST_RET > (long) wr_iovec[0].iov_len) {
		tst_res(TFAIL, "writev wrote more than expected");
		SAFE_CLOSE(fd);
		return;
	}

	/* file content matches written bytes */
	SAFE_LSEEK(fd, initial_file_offset, SEEK_SET);
	SAFE_READ(1, fd, tmp, TST_RET);
	if (memcmp(tmp, wr_iovec[0].iov_base, TST_RET) == 0) {
		tst_res(TPASS, "file has expected content");
	} else {
		tst_res(TFAIL, "file has unexpected content");
		tst_res_hexd(TFAIL, wr_iovec[0].iov_base, TST_RET,
				"expected:");
		tst_res_hexd(TFAIL, tmp, TST_RET,
				"actual file content:");
	}

	/* file offset has been updated according to written bytes */
	if (off_after == initial_file_offset + TST_RET)
		tst_res(TPASS, "offset at %ld as expected", off_after);
	else
		tst_res(TFAIL, "offset unexpected %ld", off_after);

	SAFE_CLOSE(fd);
}

static void test_writev(void)
{
	test_partially_valid_iovec(0);
	test_partially_valid_iovec(CHUNK + 1);
	test_partially_valid_iovec(getpagesize());
	test_partially_valid_iovec(getpagesize() + 1);
}

static void setup(void)
{
	bad_addr = SAFE_MMAP(0, 1, PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS,
			0, 0);
}

static struct tst_test test = {
	.needs_tmpdir = 1,
	.setup = setup,
	.test_all = test_writev,
};
