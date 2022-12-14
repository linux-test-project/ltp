// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2019 SUSE LLC
 * Author: Jorik Cronenberg <jcronenberg@suse.de>
 *
 * Test vmsplice() to a full pipe with SPLICE_F_NONBLOCK and without.
 *
 * With SPLICE_F_NONBLOCK vmsplice() should return with errno EAGAIN
 * Without SPLICE_F_NONBLOCK it should block.
 */

#define _GNU_SOURCE

#include "tst_test.h"
#include "lapi/vmsplice.h"
#include "lapi/fcntl.h"
#include <stdlib.h>

static int pipes[2];
static ssize_t pipe_max_size;
static char *write_buffer;
static struct iovec iov;

static void vmsplice_test(void)
{
	int status;
	int pid;

	TEST(vmsplice(pipes[1], &iov, 1, SPLICE_F_NONBLOCK));

	if (TST_RET < 0 && TST_ERR == EAGAIN) {
		tst_res(TPASS | TTERRNO,
		    "vmsplice(..., SPLICE_F_NONBLOCK) failed as expected");
	} else if (TST_RET < 0) {
		tst_res(TFAIL | TTERRNO,
		    "vmsplice(..., SPLICE_F_NONBLOCK) shall fail with EAGAIN");
	} else {
		tst_res(TFAIL,
		    "vmsplice(..., SPLICE_F_NONBLOCK) wrote to a full pipe");
	}

	pid = SAFE_FORK();
	if (!pid) {
		TEST(vmsplice(pipes[1], &iov, 1, 0));
		if (TST_RET < 0)
			tst_res(TFAIL | TTERRNO, "vmsplice(..., 0) failed");
		else
			tst_res(TFAIL,
			    "vmsplice(..., 0) wrote to a full pipe");
		exit(0);
	}

	if (TST_PROCESS_STATE_WAIT(pid, 'S', 1000) < 0)
		return;
	else
		tst_res(TPASS, "vmsplice(..., 0) blocked");

	SAFE_KILL(pid, SIGKILL);
	SAFE_WAIT(&status);
}

static void cleanup(void)
{
	if (pipes[1] > 0)
		SAFE_CLOSE(pipes[1]);
	if (pipes[0] > 0)
		SAFE_CLOSE(pipes[0]);
}

static void setup(void)
{
	SAFE_PIPE(pipes);

	pipe_max_size = SAFE_FCNTL(pipes[1], F_GETPIPE_SZ);
	write_buffer = tst_alloc(pipe_max_size);

	iov.iov_base = write_buffer;
	iov.iov_len = pipe_max_size;

	TEST(vmsplice(pipes[1], &iov, 1, 0));
	if (TST_RET < 0) {
		tst_brk(TBROK | TTERRNO,
		    "Initial vmsplice() to fill pipe failed");
	}
}

static struct tst_test test = {
	.setup = setup,
	.cleanup = cleanup,
	.test_all = vmsplice_test,
	.forks_child = 1,
};
