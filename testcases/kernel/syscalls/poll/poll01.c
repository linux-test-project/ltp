/*
 * Copyright (c) International Business Machines  Corp., 2001
 *	07/2001 Ported by Wayne Boyer
 * Copyright (C) 2015 Cyril Hrubis <chrubis@suse.cz>
 *
 * This program is free software;  you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY;  without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
 * the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program;  if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

/*
 * Check that poll() works for POLLOUT and POLLIN and that revents is set
 * correctly.
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

	TEST(poll(outfds, 1, -1));

	if (TEST_RETURN == -1) {
		tst_res(TFAIL | TTERRNO, "poll() POLLOUT failed");
		return;
	}

	if (outfds[0].revents != POLLOUT) {
		tst_res(TFAIL | TTERRNO, "poll() failed to set POLLOUT");
		return;
	}

	tst_res(TPASS, "poll() POLLOUT");
}

static void verify_pollin(void)
{
	char write_buf[] = "Testing";
	char read_buf[BUF_SIZE];

	struct pollfd infds[] = {
		{.fd = fildes[0], .events = POLLIN},
	};

	SAFE_WRITE(1, fildes[1], write_buf, sizeof(write_buf));

	TEST(poll(infds, 1, -1));

	if (TEST_RETURN == -1) {
		tst_res(TFAIL | TTERRNO, "poll() POLLIN failed");
		goto end;
	}

	if (infds[0].revents != POLLIN) {
		tst_res(TFAIL, "poll() failed to set POLLIN");
		goto end;
	}


	tst_res(TPASS, "poll() POLLIN");

end:
	SAFE_READ(1, fildes[0], read_buf, sizeof(write_buf));
}

void verify_poll(unsigned int n)
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
