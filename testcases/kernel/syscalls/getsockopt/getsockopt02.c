// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2017 Red Hat, Inc.
 */

/*
 * Test description: Test retrieving of peer credentials (SO_PEERCRED)
 *
 */

#define _GNU_SOURCE

#include <errno.h>
#include <stdlib.h>
#include "tst_test.h"

static int socket_fd, accepted;
static struct sockaddr_un sun;

#define SOCKNAME	"testsocket"

static void setup(void)
{
	sun.sun_family = AF_UNIX;
	(void)strcpy(sun.sun_path, SOCKNAME);
	socket_fd = SAFE_SOCKET(sun.sun_family, SOCK_STREAM, 0);
	SAFE_BIND(socket_fd, (struct sockaddr *)&sun, sizeof(sun));
	SAFE_LISTEN(socket_fd, SOMAXCONN);
}

static void fork_func(void)
{
	int fork_socket_fd = SAFE_SOCKET(sun.sun_family, SOCK_STREAM, 0);

	SAFE_CONNECT(fork_socket_fd, (struct sockaddr *)&sun, sizeof(sun));
	TST_CHECKPOINT_WAIT(0);
	SAFE_CLOSE(fork_socket_fd);
	exit(0);
}

static void test_function(void)
{
	pid_t fork_id;
	struct ucred cred;
	socklen_t cred_len = sizeof(cred);

	fork_id = SAFE_FORK();
	if (!fork_id)
		fork_func();

	accepted = accept(socket_fd, NULL, NULL);
	if (accepted < 0) {
		tst_res(TFAIL | TERRNO, "Error with accepting connection");
		goto clean;
	}
	if (getsockopt(accepted, SOL_SOCKET,
				SO_PEERCRED, &cred, &cred_len) < 0) {
		tst_res(TFAIL | TERRNO, "Error while getting socket option");
		goto clean;
	}

	if (fork_id != cred.pid)
		tst_res(TFAIL, "Received wrong PID %d, expected %d",
				cred.pid, getpid());
	else
		tst_res(TPASS, "Test passed");
clean:
	if (accepted >= 0)
		SAFE_CLOSE(accepted);
	TST_CHECKPOINT_WAKE(0);
}

static void cleanup(void)
{
	if (accepted >= 0)
		SAFE_CLOSE(accepted);
	if (socket_fd >= 0)
		SAFE_CLOSE(socket_fd);
}

static struct tst_test test = {
	.test_all = test_function,
	.setup = setup,
	.cleanup = cleanup,
	.forks_child = 1,
	.needs_checkpoints = 1,
};
