// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * This test is based on source contained in the man pages for sendmmsg and
 * recvmmsg in release 4.15 of the Linux man-pages project.
 */

#define _GNU_SOURCE
#include <netinet/ip.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#include "time64_variants.h"
#include "tst_test.h"
#include "lapi/socket.h"
#include "tst_safe_macros.h"
#include "sendmmsg_var.h"

#define BUFSIZE 16
#define VLEN 2

static int send_sockfd;
static int receive_sockfd;
static struct mmsghdr *snd_msg, *rcv_msg;
static struct iovec *snd1, *snd2, *rcv1, *rcv2;

static struct time64_variants variants[] = {
	{ .recvmmsg = libc_recvmmsg, .sendmmsg = libc_sendmmsg, .ts_type = TST_LIBC_TIMESPEC, .desc = "vDSO or syscall with libc spec"},

#if (__NR_recvmmsg != __LTP__NR_INVALID_SYSCALL)
	{ .recvmmsg = sys_recvmmsg, .sendmmsg = sys_sendmmsg, .ts_type = TST_KERN_OLD_TIMESPEC, .desc = "syscall with old kernel spec"},
#endif

#if (__NR_recvmmsg_time64 != __LTP__NR_INVALID_SYSCALL)
	{ .recvmmsg = sys_recvmmsg64, .sendmmsg = sys_sendmmsg, .ts_type = TST_KERN_TIMESPEC, .desc = "syscall time64 with kernel spec"},
#endif
};

static void run(void)
{
	struct time64_variants *tv = &variants[tst_variant];
	struct tst_ts timeout;
	int retval;

	retval = tv->sendmmsg(send_sockfd, snd_msg, VLEN, 0);
	if (retval < 0 || snd_msg[0].msg_len != 6 || snd_msg[1].msg_len != 6) {
		tst_res(TFAIL | TERRNO, "sendmmsg() failed");
		return;
	}

	memset(rcv1->iov_base, 0, rcv1->iov_len);
	memset(rcv2->iov_base, 0, rcv2->iov_len);

	timeout.type = tv->ts_type;
	tst_ts_set_sec(&timeout, 1);
	tst_ts_set_nsec(&timeout, 0);

	retval = tv->recvmmsg(receive_sockfd, rcv_msg, VLEN, 0, tst_ts_get(&timeout));

	if (retval == -1) {
		tst_res(TFAIL | TERRNO, "recvmmsg() failed");
		return;
	}
	if (retval != 2) {
		tst_res(TFAIL, "Received unexpected number of messages (%d)",
			retval);
		return;
	}

	if (memcmp(rcv1->iov_base, "onetwo", 6))
		tst_res(TFAIL, "Error in first received message");
	else
		tst_res(TPASS, "First message received successfully");

	if (memcmp(rcv2->iov_base, "three", 5))
		tst_res(TFAIL, "Error in second received message");
	else
		tst_res(TPASS, "Second message received successfully");
}

static void setup(void)
{
	struct sockaddr_in addr;
	unsigned int port = TST_GET_UNUSED_PORT(AF_INET, SOCK_DGRAM);

	send_sockfd = SAFE_SOCKET(AF_INET, SOCK_DGRAM, 0);
	receive_sockfd = SAFE_SOCKET(AF_INET, SOCK_DGRAM, 0);

	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
	addr.sin_port = port;

	SAFE_BIND(receive_sockfd, (struct sockaddr *)&addr, sizeof(addr));
	SAFE_CONNECT(send_sockfd, (struct sockaddr *)&addr, sizeof(addr));

	memcpy(snd1[0].iov_base, "one", snd1[0].iov_len);
	memcpy(snd1[1].iov_base, "two", snd1[1].iov_len);
	memcpy(snd2->iov_base, "three3", snd2->iov_len);

	memset(snd_msg, 0, VLEN * sizeof(*snd_msg));
	snd_msg[0].msg_hdr.msg_iov = snd1;
	snd_msg[0].msg_hdr.msg_iovlen = 2;
	snd_msg[1].msg_hdr.msg_iov = snd2;
	snd_msg[1].msg_hdr.msg_iovlen = 1;

	memset(rcv_msg, 0, VLEN * sizeof(*rcv_msg));
	rcv_msg[0].msg_hdr.msg_iov = rcv1;
	rcv_msg[0].msg_hdr.msg_iovlen = 1;
	rcv_msg[1].msg_hdr.msg_iov = rcv2;
	rcv_msg[1].msg_hdr.msg_iovlen = 1;

	tst_res(TINFO, "Testing variant: %s", variants[tst_variant].desc);
}

static void cleanup(void)
{
	if (send_sockfd > 0)
		SAFE_CLOSE(send_sockfd);
	if (receive_sockfd > 0)
		SAFE_CLOSE(receive_sockfd);
}

static struct tst_test test = {
	.test_all = run,
	.setup = setup,
	.cleanup = cleanup,
	.test_variants = ARRAY_SIZE(variants),
	.bufs = (struct tst_buffers []) {
		{&snd1, .iov_sizes = (int[]){3, 3, -1}},
		{&snd2, .iov_sizes = (int[]){6, -1}},
		{&rcv1, .iov_sizes = (int[]){6, -1}},
		{&rcv2, .iov_sizes = (int[]){5, -1}},
		{&snd_msg, .size = VLEN * sizeof(*snd_msg)},
		{&rcv_msg, .size = VLEN * sizeof(*rcv_msg)},
		{},
	}
};
