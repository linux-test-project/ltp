// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2021 FUJITSU LIMITED. All rights reserved.
 * Author: Yang Xu <xuyang2018.jy@fujitsu.com>
 */

/*\
 * Test whether the file offset are the same for both file descriptors.
 */

#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include "tst_test.h"
#include "tst_safe_macros.h"

#define WRITE_STR "abcdefg"

static int ofd = -1, nfd = 10;

static struct tcase {
	off_t offset;
	size_t exp_size;
	/* 0 - change offset before dup2, 1 - change offset after dup2 */
	int flag;
	char *exp_data;
	char *desc;
} tcases[] = {
	{1, 6, 0, "bcdefg", "Test offset with lseek before dup2"},
	{2, 5, 1, "cdefg", "Test offset with lseek after dup2"},
};

static void setup(void)
{
	ofd = SAFE_OPEN("testfile", O_RDWR | O_CREAT, 0644);
	SAFE_WRITE(SAFE_WRITE_ALL, ofd, WRITE_STR, sizeof(WRITE_STR) - 1);
}

static void cleanup(void)
{
	if (ofd > 0)
		SAFE_CLOSE(ofd);
	close(nfd);
}

static void run(unsigned int i)
{
	struct tcase *tc = tcases + i;
	char read_buf[20];

	memset(read_buf, 0, sizeof(read_buf));

	tst_res(TINFO, "%s", tc->desc);
	if (!tc->flag)
		SAFE_LSEEK(ofd, tc->offset, SEEK_SET);

	TEST(dup2(ofd, nfd));
	if (TST_RET == -1) {
		tst_res(TFAIL | TTERRNO, "call failed unexpectedly");
		return;
	}
	if (tc->flag)
		SAFE_LSEEK(ofd, tc->offset, SEEK_SET);

	SAFE_READ(1, nfd, read_buf, tc->exp_size);
	if (strncmp(read_buf, tc->exp_data, tc->exp_size))
		tst_res(TFAIL, "Expect %s, but get %s.", tc->exp_data, read_buf);
	else
		tst_res(TPASS, "Get expected buf %s", read_buf);
}

static struct tst_test test = {
	.needs_tmpdir = 1,
	.tcnt = ARRAY_SIZE(tcases),
	.test = run,
	.setup = setup,
	.cleanup = cleanup,
};
