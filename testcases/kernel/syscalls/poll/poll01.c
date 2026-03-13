// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines Corp., 2001
 * Ported to LTP: Wayne Boyer
 * Copyright (C) 2015 Cyril Hrubis <chrubis@suse.cz>
 */

/*\
 * Check that :manpage:`poll(2)` works for POLLOUT and POLLIN and that revents
 * is set correctly.
 */

#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/poll.h>
#include "tst_test.h"

#define BUF_SIZE	512

static int fildes[2];

static void verify_pollout(void)
{
	struct pollfd outfds[] = {
		{.fd = fildes[1], .events = POLLOUT},
	};

	TST_EXP_VAL(poll(outfds, 1, -1), 1);
	if (!TST_PASS)
		return;

	TST_EXP_EXPR(outfds[0].revents & POLLOUT);
	TST_EXP_EXPR((outfds[0].revents & ~POLLOUT) == 0);
}

static void verify_pollin(void)
{
	char write_buf[] = "Testing";
	char read_buf[BUF_SIZE];

	struct pollfd infds[] = {
		{.fd = fildes[0], .events = POLLIN},
	};

	SAFE_WRITE(SAFE_WRITE_ALL, fildes[1], write_buf, sizeof(write_buf));

	TST_EXP_VAL(poll(infds, 1, -1), 1);
	if (!TST_PASS)
		goto end;

	TST_EXP_EXPR(infds[0].revents & POLLIN);
	TST_EXP_EXPR((infds[0].revents & ~POLLIN) == 0);

end:
	SAFE_READ(1, fildes[0], read_buf, sizeof(write_buf));
}

static void verify_poll(unsigned int n)
{
	switch (n) {
	case 0:
		verify_pollout();
	break;
	case 1:
		verify_pollin();
	break;
	}
}

static void setup(void)
{
	SAFE_PIPE(fildes);
}

static void cleanup(void)
{
	if (fildes[0] > 0)
		SAFE_CLOSE(fildes[0]);

	if (fildes[1] > 0)
		SAFE_CLOSE(fildes[1]);
}

static struct tst_test test = {
	.setup = setup,
	.cleanup = cleanup,
	.test = verify_poll,
	.tcnt = 2,
};
