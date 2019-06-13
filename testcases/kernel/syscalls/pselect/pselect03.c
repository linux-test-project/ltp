// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2016 Cyril Hrubis <chrubis@suse.cz>
 */
 /*
  * This is basic test for pselect() returning without error.
  */
#include <stdio.h>
#include <fcntl.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <errno.h>

#include "tst_test.h"

static int fd;

static void verify_pselect(void)
{
	fd_set readfds;
	struct timespec tv = {0};

	FD_ZERO(&readfds);
	FD_SET(fd, &readfds);

	TEST(pselect(fd, &readfds, 0, 0, &tv, NULL));
	if (TST_RET >= 0)
		tst_res(TPASS, "pselect() succeeded retval=%li", TST_RET);
	else
		tst_res(TFAIL | TTERRNO, "pselect() failed unexpectedly");
}

static void setup(void)
{
	fd = SAFE_OPEN("pselect03_file", O_CREAT | O_RDWR, 0777);
}

static void cleanup(void)
{
	if (fd > 0)
		SAFE_CLOSE(fd);
}

static struct tst_test test = {
	.needs_tmpdir = 1,
	.setup = setup,
	.cleanup = cleanup,
	.test_all = verify_pselect,
};
