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
 * Create file with mode, dup2, [change mode], check mode
 *
 * - read only, dup2, read only ? "0444"
 * - write only, dup2, write only ? "0222"
 * - read/write, dup2 read/write ? "0666"
 * - read/write/execute, dup2, set read only, read only ? "0444"
 * - read/write/execute, dup2, set write only, write only ? "0222"
 * - read/write/execute, dup2, set read/write, read/write ? "0666"
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
	/* 0 - set mode before dup2, 1 - change mode after dup2 */
	int flag;
} tcases[] = {
	{&duprdo, 0444, 0},
	{&dupwro, 0222, 0},
	{&duprdwr, 0666, 0},
	{&duprdo, 0444, 1},
	{&dupwro, 0222, 1},
	{&duprdwr, 0666, 1},
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

	if (tc->flag)
		ofd = SAFE_CREAT(testfile, 0777);
	else
		ofd = SAFE_CREAT(testfile, tc->mode);
	nfd = *tc->nfd;

	TEST(dup2(ofd, nfd));
	if (TST_RET == -1) {
		tst_res(TFAIL | TTERRNO, "call failed unexpectedly");
		goto free;
	}
	if (tc->flag) {
		SAFE_CHMOD(testfile, tc->mode);
		tst_res(TINFO, "original mode 0777, new mode 0%o after chmod", tc->mode);
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
