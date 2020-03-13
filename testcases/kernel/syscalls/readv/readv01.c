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

#define	CHUNK		64

static char buf[CHUNK];

static struct iovec rd_iovec[] = {
	{buf, CHUNK},
	{NULL, 0},
	{NULL, 0},
};

static int fd;

static void run(void)
{
	int i, fail;
	char *vec;

	SAFE_LSEEK(fd, 0, SEEK_SET);

	if (readv(fd, rd_iovec, 0) == -1)
		tst_res(TFAIL | TERRNO, "readv failed unexpectedly");
	else
		tst_res(TPASS, "readv read 0 io vectors");

	memset(rd_iovec[0].iov_base, 0x00, CHUNK);

	if (readv(fd, rd_iovec, 3) != CHUNK) {
		tst_res(TFAIL, "readv failed reading %d bytes, "
			 "followed by two NULL vectors", CHUNK);
	} else {
		fail = 0;
		vec = rd_iovec[0].iov_base;

		for (i = 0; i < CHUNK; i++) {
			if (vec[i] != 0x42)
				fail++;
		}

		if (fail)
			tst_res(TFAIL, "Wrong buffer content");
		else
			tst_res(TPASS, "readv passed reading %d bytes "
			         "followed by two NULL vectors", CHUNK);
	}
}

static void setup(void)
{
	memset(buf, 0x42, sizeof(buf));

	fd = SAFE_OPEN("data_file", O_WRONLY | O_CREAT, 0666);
	SAFE_WRITE(1, fd, buf, sizeof(buf));
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
	.test_all = run,
	.needs_tmpdir = 1,
};
