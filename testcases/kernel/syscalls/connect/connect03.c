// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2026 Linux Test Project
 */

/*\
 * Verify that :manpage:`connect(2)` with AF_UNIX fails with -1 and sets proper errno:
 *
 * - EPROTOTYPE: The socket type does not support the protocol (e.g.,
 *   connecting a UNIX domain datagram socket to a stream socket)
 * - EACCES: Write permission is denied on the socket file
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <pwd.h>
#include <string.h>

#include "tst_test.h"

#define SOCK_FILE "sock_file"

static int fd_unix_server = -1;
static int fd_unix_dgram = -1;
static int fd_unix_stream = -1;

static struct sockaddr_un sock_un;

static struct test_case_t {
	int *fd;
	int exp_errno;
	mode_t mode;
	const char *desc;
} tcases[] = {
	{
		.fd = &fd_unix_dgram,
		.exp_errno = EPROTOTYPE,
		.mode = 0700,
		.desc = "socket type does not support the protocol"
	},
	{
		.fd = &fd_unix_stream,
		.exp_errno = EACCES,
		.mode = 0500,
		.desc = "write permission is denied on the socket file"
	},
};

static void setup(void)
{
	struct passwd *pw;

	/* Drop privileges to 'nobody' if we are running as root */
	if (geteuid() == 0) {
		pw = SAFE_GETPWNAM("nobody");
		SAFE_SETEUID(pw->pw_uid);
	}

	sock_un.sun_family = AF_UNIX;
	strncpy(sock_un.sun_path, SOCK_FILE, sizeof(sock_un.sun_path));

	fd_unix_server = SAFE_SOCKET(AF_UNIX, SOCK_STREAM, 0);
	SAFE_BIND(fd_unix_server, (struct sockaddr *)&sock_un, sizeof(sock_un));
	SAFE_CHMOD(SOCK_FILE, 0700);
	SAFE_LISTEN(fd_unix_server, 5);

	fd_unix_dgram = SAFE_SOCKET(AF_UNIX, SOCK_DGRAM, 0);
	fd_unix_stream = SAFE_SOCKET(AF_UNIX, SOCK_STREAM, 0);
}

static void cleanup(void)
{
	if (fd_unix_dgram != -1)
		SAFE_CLOSE(fd_unix_dgram);
	if (fd_unix_stream != -1)
		SAFE_CLOSE(fd_unix_stream);
	if (fd_unix_server != -1)
		SAFE_CLOSE(fd_unix_server);
}

static void verify_connect(unsigned int i)
{
	struct test_case_t *tc = &tcases[i];

	SAFE_CHMOD(SOCK_FILE, tc->mode);

	TST_EXP_FAIL(connect(*tc->fd, (const struct sockaddr *)&sock_un, sizeof(sock_un)),
		     tc->exp_errno, "%s", tc->desc);
}

static struct tst_test test = {
	.setup = setup,
	.cleanup = cleanup,
	.tcnt = ARRAY_SIZE(tcases),
	.test = verify_connect,
	.needs_tmpdir = 1,
};
