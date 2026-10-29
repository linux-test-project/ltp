// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines Corp., 2001
 * Ported to LTP: Wayne Boyer
 */

/*\
 * 1. :manpage:`creat(2)` a file using 0444 mode, write to the fildes, write
 *    should return a positive count.
 *
 * 2. :manpage:`creat(2)` should truncate a file to 0 bytes if it already
 *    exists, and should not fail.
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>

#include "tst_test.h"

static char filename[40];
static int fd;

static void setup(void)
{
	sprintf(filename, "creat01.%d", getpid());
}

static struct tcase {
	int mode;
} tcases[] = {
	{0644},
	{0444}
};

static void verify_creat(unsigned int i)
{
	struct stat buf;
	char c;

	fd = SAFE_CREAT(filename, tcases[i].mode);

	SAFE_STAT(filename, &buf);

	if (buf.st_size != 0)
		tst_res(TFAIL, "creat() failed to truncate file to 0 bytes");
	else
		tst_res(TPASS, "creat() truncated file to 0 bytes");

	if (write(fd, "A", 1) != 1)
		tst_res(TFAIL | TERRNO, "write was unsuccessful");
	else
		tst_res(TPASS, "file was created and written to successfully");

	if (read(fd, &c, 1) != -1)
		tst_res(TFAIL, "read succeeded unexpectedly");
	else
		tst_res(TPASS | TERRNO, "read failed expectedly");

	SAFE_CLOSE(fd);
}

static void cleanup(void)
{
	if (fd > 0)
		SAFE_CLOSE(fd);
}

static struct tst_test test = {
	.tcnt = 2,
	.test = verify_creat,
	.needs_tmpdir = 1,
	.cleanup = cleanup,
	.setup = setup,
};
