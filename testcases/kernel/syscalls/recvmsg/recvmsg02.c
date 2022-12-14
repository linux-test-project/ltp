// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2016 Cyril Hrubis <chrubis@suse.cz>
 * Copyright (c) 2016 Michal Kubecek <mkubecek@suse.cz>
 */

/*
 * This is a regression test for kernel commit:
 *
 * 197c949e7798  udp: properly support MSG_PEEK with truncated buffers
 *
 * NOTE: The testcase will hang on upatched stable kernel.
 */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "tst_test.h"
#include "lapi/socket.h"

static const char msg[] = "Michael Gilfix was here\341\210\264\r\n";
static const unsigned msglen = ARRAY_SIZE(msg) - 1;
static unsigned char buff[25];
static const int bufflen = ARRAY_SIZE(buff);

static int sdr, sdw;

static void verify_recvmsg(void)
{
	struct sockaddr_in6 addr_init = {
		.sin6_family	= AF_INET6,
		.sin6_port	= htons(0),
		.sin6_addr	= IN6ADDR_LOOPBACK_INIT,
	};
	struct sockaddr_in6 addr_r, addr_w, addr_f;
	socklen_t addrlen_r, addrlen_w;
	struct iovec iov = {
		.iov_base	= buff,
		.iov_len	= sizeof(buff),
	};
	struct msghdr msghdr = {
		.msg_name	= &addr_f,
		.msg_namelen	= sizeof(addr_f),
		.msg_iov	= &iov,
		.msg_iovlen	= 1,
		.msg_control	= NULL,
		.msg_controllen	= 0,
		.msg_flags	= 0,
	};
	int R;

	sdr = SAFE_SOCKET(PF_INET6, SOCK_DGRAM | SOCK_CLOEXEC, IPPROTO_IP);
	SAFE_BIND(sdr, (struct sockaddr*)&addr_init, sizeof(addr_init));
	addrlen_r = sizeof(addr_r);
	SAFE_GETSOCKNAME(sdr, (struct sockaddr*)&addr_r, &addrlen_r);
	sdw = SAFE_SOCKET(PF_INET6, SOCK_DGRAM|SOCK_CLOEXEC, IPPROTO_IP);
	SAFE_BIND(sdw, (struct sockaddr*)&addr_init, sizeof(addr_init));
	addrlen_w = sizeof(addr_w);
	SAFE_GETSOCKNAME(sdw, (struct sockaddr*)&addr_w, &addrlen_w);

	R = sendto(sdw, msg, msglen, 0, (struct sockaddr*)&addr_r, addrlen_r);
	if (R < 0)
		tst_brk(TBROK | TERRNO, "sendto()");

	R = recvmsg(sdr, &msghdr, MSG_PEEK);
	if (R < 0) {
		tst_res(TFAIL | TERRNO, "recvmsg(..., MSG_PEEK)");
		return;
	}

	tst_res(TINFO, "received %d bytes", R);

	if ((R == bufflen) && !memcmp(msg, buff, R))
		tst_res(TPASS, "recvmsg(..., MSG_PEEK) works fine");
	else
		tst_res(TPASS, "recvmsg(..., MSG_PEEK) failed");

	SAFE_CLOSE(sdw);
	SAFE_CLOSE(sdr);
}

static void cleanup(void)
{
	if (sdw > 0)
		SAFE_CLOSE(sdw);

	if (sdr > 0)
		SAFE_CLOSE(sdr);
}

static struct tst_test test = {
	.test_all = verify_recvmsg,
	.cleanup = cleanup,
	.tags = (const struct tst_tag[]) {
		{"linux-git", "197c949e7798"},
		{}
	}
};
