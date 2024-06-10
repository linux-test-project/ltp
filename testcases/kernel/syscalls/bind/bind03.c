// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2018 Michael Moese <mmoese@suse.com>
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
static struct sockaddr_un sun1, sun2;

static void run(void)
{
	/*
	 * Once a STREAM UNIX domain socket has been bound, it can't be
	 * rebound.
	 */
	TST_EXP_FAIL(bind(sock1, (struct sockaddr *)&sun2, sizeof(sun2)),
		     EINVAL, "re-bind() socket");

	/*
	 * Since a socket is already bound to the pathname, it can't be bound
	 * to a second socket. Expected error is EADDRINUSE.
	 */
	TST_EXP_FAIL(bind(sock2, (struct sockaddr *)&sun1, sizeof(sun1)),
		     EADDRINUSE, "bind() with bound pathname");

	/*
	 * Kernel is buggy since it creates the node in fileystem first, then
	 * locks the socket and does all the checks and the node is not removed
	 * in the error path. For now we will unlink the node here so that the
	 * test works fine when the run() function is executed in a loop.
	 * From v5.14-rc1 the kernel has fix above issue.
	 */
	if (tst_kvercmp(5, 14, 0) >= 0)
		TST_EXP_FAIL(unlink(SNAME_B), ENOENT, "check exist of SNAME_B");
	else
		unlink(SNAME_B);
}

static void setup(void)
{
	sock1 = SAFE_SOCKET(PF_UNIX, SOCK_STREAM, 0);
	sock2 = SAFE_SOCKET(PF_UNIX, SOCK_STREAM, 0);

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
}

static void cleanup(void)
{
	if (sock1 > 0)
		SAFE_CLOSE(sock1);

	if (sock2 > 0)
		SAFE_CLOSE(sock2);
}

static struct tst_test test = {
	.setup = setup,
	.cleanup = cleanup,
	.test_all = run,
	.needs_tmpdir = 1,
};
