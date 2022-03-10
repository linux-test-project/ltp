// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2019 SUSE LLC <mdoucha@suse.cz>
 */

/*
 * CVE-2017-1000112
 *
 * Check that UDP fragmentation offload doesn't cause memory corruption
 * if the userspace process turns off UFO in between two send() calls.
 * Kernel crash fixed in:
 *
 *  commit 85f1bd9a7b5a79d5baa8bf44af19658f7bf77bfa
 *  Author: Willem de Bruijn <willemb@google.com>
 *  Date:   Thu Aug 10 12:29:19 2017 -0400
 *
 *  udp: consistently apply ufo or fragmentation
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

#define BUFSIZE 4000

static struct sockaddr_in addr;
static int dst_sock = -1;

static void setup(void)
{
	int real_uid = getuid();
	int real_gid = getgid();
	struct ifreq ifr;
	socklen_t addrlen = sizeof(addr);

	SAFE_TRY_FILE_PRINTF("/proc/sys/user/max_user_namespaces", "%d", 10);

	SAFE_UNSHARE(CLONE_NEWUSER);
	SAFE_UNSHARE(CLONE_NEWNET);
	SAFE_FILE_PRINTF("/proc/self/setgroups", "deny");
	SAFE_FILE_PRINTF("/proc/self/uid_map", "0 %d 1", real_uid);
	SAFE_FILE_PRINTF("/proc/self/gid_map", "0 %d 1", real_gid);

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
		{"?/proc/sys/user/max_user_namespaces", NULL},
		NULL,
	},
	.tags = (const struct tst_tag[]) {
		{"linux-git", "85f1bd9a7b5a"},
		{"CVE", "2017-1000112"},
		{}
	}
};
