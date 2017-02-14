/*
 * Copyright(c) 2016 Fujitsu Ltd.
 * Author: Xiao Yang <yangx.jy@cn.fujitsu.com>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it would be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * You should have received a copy of the GNU General Public License
 * alone with this program.
 */

/*
 * Test Name: sendto02
 *
 * Description:
 * When sctp protocol is selected in socket(2) and buffer is invalid,
 * sendto(2) should fail and set errno to EFAULT, but it sets errno
 * to ENOMEM.
 *
 * This is a regression test and has been fixed by kernel commit:
 * 6e51fe7572590d8d86e93b547fab6693d305fd0d (sctp: fix -ENOMEM result
 * with invalid user space pointer in sendto() syscall)
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
	if (TEST_RETURN != -1) {
		tst_res(TFAIL, "sendto(fd, NULL, ...) succeeded unexpectedly");
		return;
	}

	if (TEST_ERRNO == EFAULT) {
		tst_res(TPASS | TTERRNO,
			"sendto(fd, NULL, ...) failed expectedly");
		return;
	}

	tst_res(TFAIL | TTERRNO,
		"sendto(fd, NULL, ...) failed unexpectedly, expected EFAULT");
}

static struct tst_test test = {
	.tid = "sendto02",
	.setup = setup,
	.cleanup = cleanup,
	.test_all = verify_sendto,
};
