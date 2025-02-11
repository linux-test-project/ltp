// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2020 SUSE LLC <mdoucha@suse.cz>
 */

/*\
 * Check that the kernel correctly handles send()/sendto()/sendmsg() calls
 * with MSG_MORE flag.
 */

#define _GNU_SOURCE
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <sched.h>

#include "tst_test.h"
#include "tst_net.h"

#define SENDSIZE 16
#define RECVSIZE 32

static int sock = -1, dst_sock = -1, listen_sock = -1;
static struct sockaddr_in addr;
static char sendbuf[SENDSIZE];

static void do_send(int sock, void *buf, size_t size, int flags)
{
	SAFE_SEND(1, sock, buf, size, flags);
}

static void do_sendto(int sock, void *buf, size_t size, int flags)
{
	SAFE_SENDTO(1, sock, buf, size, flags, (struct sockaddr *)&addr,
		sizeof(addr));
}

static void do_sendmsg(int sock, void *buf, size_t size, int flags)
{
	struct msghdr msg;
	struct iovec iov;

	iov.iov_base = buf;
	iov.iov_len = size;
	msg.msg_name = &addr;
	msg.msg_namelen = sizeof(addr);
	msg.msg_iov = &iov;
	msg.msg_iovlen = 1;
	msg.msg_control = NULL;
	msg.msg_controllen = 0;
	msg.msg_flags = 0;
	SAFE_SENDMSG(size, sock, &msg, flags);
}

static struct test_case {
	int domain, type, protocol;
	void (*send)(int sock, void *buf, size_t size, int flags);
	int needs_connect, needs_accept;
	const char *name;
} testcase_list[] = {
	{AF_INET, SOCK_STREAM, 0, do_send, 1, 1, "TCP send"},
	{AF_INET, SOCK_DGRAM, 0, do_send, 1, 0, "UDP send"},
	{AF_INET, SOCK_DGRAM, 0, do_sendto, 0, 0, "UDP sendto"},
	{AF_INET, SOCK_DGRAM, 0, do_sendmsg, 0, 0, "UDP sendmsg"}
};

static void setup(void)
{
	memset(sendbuf, 0x42, SENDSIZE);
}

static int check_recv(int sock, long expsize, int loop)
{
	char recvbuf[RECVSIZE] = {0};

	while (1) {
		TEST(recv(sock, recvbuf, RECVSIZE, MSG_DONTWAIT));

		if (TST_RET == -1) {
			/* expected error immediately after send(MSG_MORE) */
			if (TST_ERR == EAGAIN || TST_ERR == EWOULDBLOCK) {
				if (expsize)
					continue;
				else
					break;
			}

			/* unexpected error */
			tst_res(TFAIL | TTERRNO, "recv() error at step %d, expsize %ld",
				loop, expsize);
			return 0;
		}

		if (TST_RET < 0) {
			tst_res(TFAIL | TTERRNO, "recv() returns %ld at step %d, expsize %ld",
				TST_RET, loop, expsize);
			return 0;
		}

		if (TST_RET != expsize) {
			tst_res(TFAIL, "recv() read %ld bytes, expected %ld, step %d",
				TST_RET, expsize, loop);
			return 0;
		}
		return 1;
	}

	return 1;
}

static void cleanup(void)
{
	if (sock >= 0)
		SAFE_CLOSE(sock);

	if (dst_sock >= 0 && dst_sock != listen_sock)
		SAFE_CLOSE(dst_sock);

	if (listen_sock >= 0)
		SAFE_CLOSE(listen_sock);
}

static void run(unsigned int n)
{
	int i, ret;
	struct test_case *tc = testcase_list + n;
	socklen_t len = sizeof(addr);

	tst_res(TINFO, "Testing %s", tc->name);

	tst_init_sockaddr_inet_bin(&addr, INADDR_LOOPBACK, 0);
	listen_sock = SAFE_SOCKET(tc->domain, tc->type, tc->protocol);
	dst_sock = listen_sock;
	SAFE_BIND(listen_sock, (struct sockaddr *)&addr, sizeof(addr));
	SAFE_GETSOCKNAME(listen_sock, (struct sockaddr *)&addr, &len);

	if (tc->needs_accept)
		SAFE_LISTEN(listen_sock, 1);

	for (i = 0; i < 1000; i++) {
		sock = SAFE_SOCKET(tc->domain, tc->type, tc->protocol);

		if (tc->needs_connect)
			SAFE_CONNECT(sock, (struct sockaddr *)&addr, len);

		if (tc->needs_accept)
			dst_sock = SAFE_ACCEPT(listen_sock, NULL, NULL);

		tc->send(sock, sendbuf, SENDSIZE, 0);
		ret = check_recv(dst_sock, SENDSIZE, i + 1);

		if (!ret)
			break;

		tc->send(sock, sendbuf, SENDSIZE, MSG_MORE);
		ret = check_recv(dst_sock, 0, i + 1);

		if (!ret)
			break;

		tc->send(sock, sendbuf, 1, 0);
		ret = check_recv(dst_sock, SENDSIZE + 1, i + 1);

		if (!ret)
			break;

		SAFE_CLOSE(sock);

		if (dst_sock != listen_sock)
			SAFE_CLOSE(dst_sock);
	}

	if (ret)
		tst_res(TPASS, "MSG_MORE works correctly");

	cleanup();
	dst_sock = -1;
}

static struct tst_test test = {
	.test = run,
	.tcnt = ARRAY_SIZE(testcase_list),
	.setup = setup,
	.cleanup = cleanup
};
