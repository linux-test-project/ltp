/*
 * Copyright (c) 2018 Michael Moese <mmoese@suse.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
/* The commit 0fb44559ffd6  af_unix: move unix_mknod() out of bindlock
 * changed the behavior of bind() for STREAM UNIX domain sockets if
 */

#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "tst_kvercmp.h"
#include "tst_test.h"
#include "tst_safe_net.h"

#define SNAME_A "socket.1"
#define SNAME_B "socket.2"

static int sock1, sock2;

void run(void)
{
	struct sockaddr_un sun1;
	struct sockaddr_un sun2;

	sock1 = SAFE_SOCKET(PF_UNIX, SOCK_STREAM, 0);

	memset(&sun1, 0, sizeof(sun1));
	memset(&sun2, 0, sizeof(sun2));

	sun1.sun_family = AF_UNIX;
	sun2.sun_family = AF_UNIX;

	if (sprintf(sun1.sun_path, "%s", SNAME_A) < (int) strlen(SNAME_A)) {
		tst_res(TFAIL, "sprintf failed");
		return;
	}

	if (sprintf(sun2.sun_path, "%s", SNAME_B) < (int) strlen(SNAME_B)) {
		tst_res(TFAIL, "sprintf failed");
		return;
	}

	SAFE_BIND(sock1, (struct sockaddr *)&sun1, sizeof(sun1));

	/*
	 * Once a STREAM UNIX domain socket has been bound, it can't be
	 * rebound.
	 */
	if (bind(sock1, (struct sockaddr *)&sun2, sizeof(sun2)) == 0) {
		tst_res(TFAIL, "re-binding of socket succeeded");
		return;
	}

	if (errno != EINVAL) {
		tst_res(TFAIL | TERRNO, "expected EINVAL");
		return;
	}

	tst_res(TPASS, "bind() failed with EINVAL as expected");

	sock2 = SAFE_SOCKET(PF_UNIX, SOCK_STREAM, 0);

	/*
	 * Since a socket is already bound to the pathname, it can't be bound
	 * to a second socket. Expected error is EADDRINUSE.
	 */
	if (bind(sock2, (struct sockaddr *)&sun1, sizeof(sun1)) == 0) {
		tst_res(TFAIL, "bind() succeeded with already bound pathname!");
		return;
	}

	if (errno != EADDRINUSE) {
		tst_res(TFAIL | TERRNO, "expected to fail with EADDRINUSE");
		return;
	}

	tst_res(TPASS, "bind() failed with EADDRINUSE as expected");
}

static void cleanup(void)
{
	close(sock1);
	close(sock2);
}

static struct tst_test test = {
	.cleanup = cleanup,
	.test_all = run,
	.needs_tmpdir = 1,
};
