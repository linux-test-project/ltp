// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines  Corp., 2001
 * 07/2001 Ported by Wayne Boyer
 */

/*\
 * [Description]
 *
 * Test whether the access mode are the same for both file descriptors.
 *
 * - 0: read only ? "0444"
 * - 1: write only ? "0222"
 * - 2: read/write ? "0666"
 */

#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include "tst_test.h"
#include "tst_safe_macros.h"

static char testfile[40];
static int ofd = -1, nfd = -1;

/* set these to a known index into our local file descriptor table */
static int duprdo = 10, dupwro = 20, duprdwr = 30;

static struct tcase {
	int *nfd;
	mode_t mode;
} tcases[] = {
	{&duprdo, 0444},
	{&dupwro, 0222},
	{&duprdwr, 0666},
};

static void setup(void)
{
	umask(0);
	sprintf(testfile, "dup202.%d", getpid());
}

static void cleanup(void)
{
	close(ofd);
	close(nfd);
}

static void run(unsigned int i)
{
	struct stat oldbuf, newbuf;
	struct tcase *tc = tcases + i;

	ofd = SAFE_CREAT(testfile, tc->mode);
	nfd = *tc->nfd;

	TEST(dup2(ofd, nfd));
	if (TST_RET == -1) {
		tst_res(TFAIL | TTERRNO, "call failed unexpectedly");
		goto free;
	}

	SAFE_FSTAT(ofd, &oldbuf);

	SAFE_FSTAT(nfd, &newbuf);

	if (oldbuf.st_mode != newbuf.st_mode)
		tst_res(TFAIL, "original(%o) and duped(%o) are not same mode",
			oldbuf.st_mode, newbuf.st_mode);
	else
		tst_res(TPASS, "original(%o) and duped(%o) are the same mode",
			oldbuf.st_mode, newbuf.st_mode);

	SAFE_CLOSE(nfd);
free:
	SAFE_CLOSE(ofd);
	SAFE_UNLINK(testfile);
}

static struct tst_test test = {
	.needs_tmpdir = 1,
	.tcnt = ARRAY_SIZE(tcases),
	.test = run,
	.setup = setup,
	.cleanup = cleanup,
};
