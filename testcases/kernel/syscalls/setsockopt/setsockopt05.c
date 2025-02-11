// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2019 SUSE LLC <mdoucha@suse.cz>
 */

/*\
 * CVE-2017-1000112
 *
 * Check that UDP fragmentation offload doesn't cause memory corruption
 * if the userspace process turns off UFO in between two send() calls.
 *
 * Kernel crash fixed in 4.13
 * 85f1bd9a7b5a ("udp: consistently apply ufo or fragmentation")
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <net/if.h>

#include "tst_test.h"
#include "tst_net.h"

#define BUFSIZE 4000

static struct sockaddr_in addr;
static int dst_sock = -1;

static void setup(void)
{
	struct ifreq ifr;
	socklen_t addrlen = sizeof(addr);

	tst_setup_netns();

	tst_init_sockaddr_inet_bin(&addr, INADDR_LOOPBACK, 0);
	dst_sock = SAFE_SOCKET(AF_INET, SOCK_DGRAM, 0);

	strcpy(ifr.ifr_name, "lo");
	ifr.ifr_mtu = 1500;
	SAFE_IOCTL(dst_sock, SIOCSIFMTU, &ifr);
	ifr.ifr_flags = IFF_UP;
	SAFE_IOCTL(dst_sock, SIOCSIFFLAGS, &ifr);

	SAFE_BIND(dst_sock, (struct sockaddr *)&addr, addrlen);
	SAFE_GETSOCKNAME(dst_sock, (struct sockaddr*)&addr, &addrlen);
}

static void cleanup(void)
{
	if (dst_sock != -1)
		SAFE_CLOSE(dst_sock);
}

static void run(void)
{
	int sock, i;
	char buf[BUFSIZE];

	memset(buf, 0x42, BUFSIZE);

	for (i = 0; i < 1000; i++) {
		sock = SAFE_SOCKET(AF_INET, SOCK_DGRAM, 0);
		SAFE_CONNECT(sock, (struct sockaddr *)&addr, sizeof(addr));
		SAFE_SEND(1, sock, buf, BUFSIZE, MSG_MORE);
		SAFE_SETSOCKOPT_INT(sock, SOL_SOCKET, SO_NO_CHECK, 1);
		send(sock, buf, 1, 0);
		SAFE_CLOSE(sock);

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
	.taint_check = TST_TAINT_W | TST_TAINT_D,
	.needs_kconfigs = (const char *[]) {
		"CONFIG_USER_NS=y",
		"CONFIG_NET_NS=y",
		NULL
	},
	.save_restore = (const struct tst_path_val[]) {
		{"/proc/sys/user/max_user_namespaces", "1024", TST_SR_SKIP},
		{}
	},
	.tags = (const struct tst_tag[]) {
		{"linux-git", "85f1bd9a7b5a"},
		{"CVE", "2017-1000112"},
		{}
	}
};
