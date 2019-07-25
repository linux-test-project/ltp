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

#include "tst_test.h"
#include "lapi/socket.h"
#include "tst_safe_macros.h"

#include "sendmmsg_var.h"

#define BUFSIZE 16
#define VLEN 2

static int send_sockfd;
static int receive_sockfd;
static struct mmsghdr msg[VLEN];
static struct iovec msg1[2], msg2;

static void run(void)
{
	struct mmsghdr msgs_in[VLEN];
	struct iovec iovecs[VLEN];
	char bufs[VLEN][BUFSIZE+1];
	struct timespec timeout;
	int i, retval;

	retval = do_sendmmsg(send_sockfd, msg, VLEN, 0);
	if (retval < 0 || msg[0].msg_len != 6 || msg[1].msg_len != 5) {
		tst_res(TFAIL|TTERRNO, "sendmmsg failed");
		return;
	}

	memset(msgs_in, 0, sizeof(msgs_in));
	for (i = 0; i < VLEN; i++) {
		iovecs[i].iov_base = bufs[i];
		iovecs[i].iov_len = BUFSIZE;
		msgs_in[i].msg_hdr.msg_iov = &iovecs[i];
		msgs_in[i].msg_hdr.msg_iovlen = 1;
	}

	timeout.tv_sec = 1;
	timeout.tv_nsec = 0;

	retval = do_recvmmsg(receive_sockfd, msgs_in, VLEN, 0, &timeout);

	if (retval == -1) {
		tst_res(TFAIL | TTERRNO, "recvmmsg failed");
		return;
	}
	if (retval != 2) {
		tst_res(TFAIL, "Received unexpected number of messages (%d)",
			retval);
		return;
	}

	bufs[0][msgs_in[0].msg_len] = 0;
	if (strcmp(bufs[0], "onetwo"))
		tst_res(TFAIL, "Error in first received message");
	else
		tst_res(TPASS, "First message received successfully");

	bufs[1][msgs_in[1].msg_len] = 0;
	if (strcmp(bufs[1], "three"))
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
	SAFE_CONNECT(send_sockfd, (struct sockaddr *) &addr, sizeof(addr));

	memset(msg1, 0, sizeof(msg1));
	msg1[0].iov_base = "one";
	msg1[0].iov_len = 3;
	msg1[1].iov_base = "two";
	msg1[1].iov_len = 3;

	memset(&msg2, 0, sizeof(msg2));
	msg2.iov_base = "three";
	msg2.iov_len = 5;

	memset(msg, 0, sizeof(msg));
	msg[0].msg_hdr.msg_iov = msg1;
	msg[0].msg_hdr.msg_iovlen = 2;

	msg[1].msg_hdr.msg_iov = &msg2;
	msg[1].msg_hdr.msg_iovlen = 1;

	test_info();
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
	.test_variants = TEST_VARIANTS,
};
