// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2019 SUSE LLC
 * Author: Jorik Cronenberg <jcronenberg@suse.de>
 *
 * Test vmsplice() from a pipe into user memory
 */

#define _GNU_SOURCE

#include "tst_test.h"
#include "lapi/fcntl.h"
#include "lapi/vmsplice.h"


#define TEST_BLOCK_SIZE (64*1024)	/* 64K */

static char buffer[TEST_BLOCK_SIZE];
static struct iovec *iov;

static void vmsplice_test(void)
{
	int written, i;
	int pipes[2];
	char *arr_write = iov->iov_base;

	memset(iov->iov_base, 0, iov->iov_len);

	SAFE_PIPE(pipes);
	SAFE_WRITE(SAFE_WRITE_ALL, pipes[1], buffer, TEST_BLOCK_SIZE);
	written = vmsplice(pipes[0], iov, 1, 0);

	if (written < 0)
		tst_brk(TBROK | TERRNO, "vmsplice() failed");

	if (written == 0) {
		tst_res(TFAIL, "vmsplice() didn't write anything");
	} else {
		for (i = 0; i < TEST_BLOCK_SIZE; i++) {
			if (arr_write[i] != buffer[i]) {
				tst_res(TFAIL,
					"Wrong data in user memory at %i", i);
				break;
			}
		}
		if (i == written)
			tst_res(TPASS, "Spliced correctly into user memory");
	}

	SAFE_CLOSE(pipes[1]);
	SAFE_CLOSE(pipes[0]);
}

static void setup(void)
{
	int i;

	for (i = 0; i < TEST_BLOCK_SIZE; i++)
		buffer[i] = i & 0xff;
}

static struct tst_test test = {
	.setup = setup,
	.test_all = vmsplice_test,
	.bufs = (struct tst_buffers []) {
		{&iov, .iov_sizes = (int[]){TEST_BLOCK_SIZE, -1}},
		{}
	}
};
