// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright(c) 2016 Fujitsu Ltd.
 * Author: Xiao Yang <yangx.jy@cn.fujitsu.com>
 * Copyright (c) Linux Test Project, 2017-2019
 */

/*\
 * When SCTP protocol created wih socket(2) and buffer is invalid,
 * sendto(2) should fail and set errno to EFAULT, but it sets errno
 * to ENOMEM.
 *
 * This is a regression test fixed by kernel 3.7
 * 6e51fe757259 (sctp: fix -ENOMEM result with invalid user space pointer in sendto() syscall)
 */

#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "tst_test.h"

#ifndef IPPROTO_SCTP
# define IPPROTO_SCTP	132
#endif

static int sockfd;
static struct sockaddr_in sa;

static void setup(void)
{
	sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_SCTP);
	if (sockfd == -1) {
		if (errno == EPROTONOSUPPORT)
			tst_brk(TCONF, "sctp protocol was not supported");
		else
			tst_brk(TBROK | TERRNO, "socket() failed with sctp");
	}

	memset(&sa, 0, sizeof(sa));
	sa.sin_family = AF_INET;
	sa.sin_addr.s_addr = inet_addr("127.0.0.1");
	sa.sin_port = htons(11111);
}

static void cleanup(void)
{
	if (sockfd > 0)
		SAFE_CLOSE(sockfd);
}

static void verify_sendto(void)
{
	TEST(sendto(sockfd, NULL, 1, 0, (struct sockaddr *) &sa, sizeof(sa)));
	if (TST_RET != -1) {
		tst_res(TFAIL, "sendto(fd, NULL, ...) succeeded unexpectedly");
		return;
	}

	if (TST_ERR == EFAULT) {
		tst_res(TPASS | TTERRNO,
			"sendto(fd, NULL, ...) failed expectedly");
		return;
	}

	tst_res(TFAIL | TTERRNO,
		"sendto(fd, NULL, ...) failed unexpectedly, expected EFAULT");
}

static struct tst_test test = {
	.setup = setup,
	.cleanup = cleanup,
	.test_all = verify_sendto,
	.tags = (const struct tst_tag[]) {
		{"linux-git", "6e51fe757259"},
		{}
	}
};
