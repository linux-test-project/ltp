// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2024 SUSE LLC Andrea Cervesato <andrea.cervesato@suse.com>
 */

/*\
 * This test verifies the following shutdown() errors:
 *
 * - EBADF sockfd is not a valid file descriptor
 * - EINVAL An invalid value was specified in how
 * - ENOTCONN The specified socket is not connected
 * - ENOTSOCK The file descriptor sockfd does not refer to a socket
 */

#include "tst_test.h"

static int file_desc;
static int valid_sock;
static int invalid_sock = -1;

static struct sockaddr_in *server_addr;

static struct tcase {
	int *socket;
	int flags;
	int error;
} tcases[] = {
	{&invalid_sock, PF_INET, EBADF},
	{&valid_sock,   -1,      EINVAL},
	{&valid_sock,   PF_INET, ENOTCONN},
	{&file_desc,    PF_INET, ENOTSOCK},
};

static void run(unsigned int n)
{
	struct tcase *tc = &tcases[n];

	TST_EXP_FAIL(shutdown(*tc->socket, tc->flags), tc->error);
}

static void setup(void)
{
	file_desc = SAFE_OPEN("notasocket", O_CREAT, 0640);
	valid_sock = SAFE_SOCKET(PF_INET, SOCK_STREAM, 0);

	server_addr->sin_family = AF_INET;
	server_addr->sin_port = 0;
	server_addr->sin_addr.s_addr = INADDR_ANY;

	SAFE_BIND(valid_sock,
		(struct sockaddr *)server_addr,
		sizeof(struct sockaddr_in));
}

static void cleanup(void)
{
	if (valid_sock > 0)
		SAFE_CLOSE(valid_sock);

	if (file_desc > 0)
		SAFE_CLOSE(file_desc);
}

static struct tst_test test = {
	.test = run,
	.tcnt = ARRAY_SIZE(tcases),
	.setup = setup,
	.cleanup = cleanup,
	.needs_tmpdir = 1,
	.bufs = (struct tst_buffers []) {
		{&server_addr, .size = sizeof(struct sockaddr_in)},
		{}
	}
};
