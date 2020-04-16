// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2017 Christoph Paasch <cpaasch@apple.com>
 * Copyright (C) 2020 SUSE LLC <mdoucha@suse.cz>
 *
 * CVE-2018-9568
 *
 * Test that connect() to AF_UNSPEC address correctly converts IPV6 socket
 * to IPV4 listen socket when IPV6_ADDRFORM is set to AF_INET.
 * Kernel memory corruption fixed in:
 *
 *  commit 9d538fa60bad4f7b23193c89e843797a1cf71ef3
 *  Author: Christoph Paasch <cpaasch@apple.com>
 *  Date:   Tue Sep 26 17:38:50 2017 -0700
 *
 *  net: Set sk_prot_creator when cloning sockets to the right proto
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>

#include "tst_test.h"
#include "tst_net.h"
#include "tst_taint.h"

static int listenfd;
static struct sockaddr_in6 bind_addr;
static struct sockaddr_in bind_addr4, client_addr;
static struct sockaddr reset_addr;

static void setup(void)
{
	socklen_t size = sizeof(bind_addr);

	tst_taint_init(TST_TAINT_W | TST_TAINT_D);

	tst_init_sockaddr_inet6_bin(&bind_addr, &in6addr_any, 0);
	tst_init_sockaddr_inet_bin(&bind_addr4, INADDR_ANY, 0);
	memset(&reset_addr, 0, sizeof(reset_addr));
	reset_addr.sa_family = AF_UNSPEC;

	listenfd = SAFE_SOCKET(AF_INET6, SOCK_STREAM, IPPROTO_TCP);
	SAFE_BIND(listenfd, (struct sockaddr *)&bind_addr, sizeof(bind_addr));
	SAFE_LISTEN(listenfd, 5);
	SAFE_GETSOCKNAME(listenfd, (struct sockaddr *)&bind_addr, &size);
	tst_init_sockaddr_inet(&client_addr, "127.0.0.1",
		htons(bind_addr.sin6_port));
}

static void cleanup(void)
{
	if (listenfd > 0)
		SAFE_CLOSE(listenfd);
}

static void run(void)
{
	int i, addrlen, fd, confd1, confd2, confd3;
	struct sockaddr_storage client_addr2;

	for (i = 0; i < 1000; i++) {
		confd1 = SAFE_SOCKET(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		SAFE_CONNECT(confd1, (struct sockaddr *)&client_addr,
			sizeof(client_addr));

		fd = SAFE_ACCEPT(listenfd, NULL, NULL);
		SAFE_SETSOCKOPT_INT(fd, SOL_IPV6, IPV6_ADDRFORM, AF_INET);
		SAFE_CONNECT(fd, (struct sockaddr *)&reset_addr,
			sizeof(reset_addr));
		SAFE_BIND(fd, (struct sockaddr *)&bind_addr4,
			sizeof(bind_addr4));
		SAFE_LISTEN(fd, 5);

		addrlen = tst_get_connect_address(fd, &client_addr2);
		confd2 = SAFE_SOCKET(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		SAFE_CONNECT(confd2, (struct sockaddr *)&client_addr2, addrlen);
		confd3 = SAFE_ACCEPT(fd, NULL, NULL);

		SAFE_CLOSE(confd3);
		SAFE_CLOSE(confd2);
		SAFE_CLOSE(confd1);
		SAFE_CLOSE(fd);

		if (tst_taint_check()) {
			tst_res(TFAIL, "Kernel is vulnerable");
			return;
		}
	}

	tst_res(TPASS, "Nothing bad happened, probably");
}

static struct tst_test test = {
	.test_all = run,
	.setup = setup,
	.cleanup = cleanup,
	.tags = (const struct tst_tag[]) {
		{"linux-git", "9d538fa60bad"},
		{"CVE", "2018-9568"},
		{}
	}
};
