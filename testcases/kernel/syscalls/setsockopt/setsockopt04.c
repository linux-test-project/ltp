// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2019 SUSE LLC
 * Author: Christian Amann <camann@suse.com>
 */
/* Test for CVE-2016-9793
 *
 * With kernels between version 3.11 and 4.8 missing commit b98b0bc8 it
 * is possible to pass a very high unsigned integer as send buffer size
 * to a socket which is then interpreted as a negative value.
 *
 * This can be used to escalate privileges by every user that has the
 * CAP_NET_ADMIN capability.
 *
 * For additional information about this CVE see:
 * https://www.suse.com/security/cve/CVE-2016-9793/
 */

#include <sys/socket.h>
#include "tst_test.h"
#include "tst_safe_net.h"

#define SNDBUF	(0xffffff00)

static int sockfd;

static void run(void)
{
	unsigned int sndbuf, rec_sndbuf;
	socklen_t optlen;

	sndbuf = SNDBUF;
	rec_sndbuf = 0;
	optlen = sizeof(sndbuf);

	SAFE_SETSOCKOPT(sockfd, SOL_SOCKET, SO_SNDBUFFORCE, &sndbuf, optlen);
	SAFE_GETSOCKOPT(sockfd, SOL_SOCKET, SO_SNDBUF, &rec_sndbuf, &optlen);

	tst_res(TINFO, "Try to set send buffer size to: %u", sndbuf);
	tst_res(TINFO, "Send buffer size was set to: %d", rec_sndbuf);

	if ((int)rec_sndbuf < 0)
		tst_res(TFAIL, "Was able to set negative send buffer size!");
	else
		tst_res(TPASS, "Was unable to set negative send buffer size!");
}

static void setup(void)
{
	sockfd = SAFE_SOCKET(AF_INET, SOCK_DGRAM, 0);
}

static void cleanup(void)
{
	if (sockfd > 0)
		SAFE_CLOSE(sockfd);
}

static struct tst_test test = {
	.test_all = run,
	.setup = setup,
	.cleanup = cleanup,
	.needs_root = 1,
	.tags = (const struct tst_tag[]) {
		{"linux-git", "b98b0bc8c431"},
		{"CVE", "2016-9793"},
		{}
	}
};
