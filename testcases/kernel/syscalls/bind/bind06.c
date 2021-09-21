// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2020 SUSE LLC <mdoucha@suse.cz>
 *
 * CVE-2018-18559
 *
 * Test for race condition vulnerability in bind() on AF_PACKET socket.
 * Fixed in:
 *
 *  commit 15fe076edea787807a7cdc168df832544b58eba6
 *  Author: Eric Dumazet <edumazet@google.com>
 *  Date:   Tue Nov 28 08:03:30 2017 -0800
 *
 *  net/packet: fix a race in packet_bind() and packet_notifier()
 */

#define _GNU_SOURCE
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <linux/if_packet.h>
#include <net/ethernet.h>
#include <net/if.h>
#include <sched.h>
#include "tst_test.h"
#include "tst_fuzzy_sync.h"

static volatile int fd = -1;
static struct sockaddr_ll addr1, addr2;
static struct tst_fzsync_pair fzsync_pair;

static void setup(void)
{
	int real_uid = getuid();
	int real_gid = getgid();
	struct ifreq ifr;

	SAFE_TRY_FILE_PRINTF("/proc/sys/user/max_user_namespaces", "%d", 10);

	SAFE_UNSHARE(CLONE_NEWUSER);
	SAFE_UNSHARE(CLONE_NEWNET);
	SAFE_FILE_PRINTF("/proc/self/setgroups", "deny");
	SAFE_FILE_PRINTF("/proc/self/uid_map", "0 %d 1\n", real_uid);
	SAFE_FILE_PRINTF("/proc/self/gid_map", "0 %d 1\n", real_gid);

	fd = SAFE_SOCKET(AF_PACKET, SOCK_DGRAM, PF_PACKET);
	strcpy(ifr.ifr_name, "lo");
	SAFE_IOCTL(fd, SIOCGIFINDEX, &ifr);
	SAFE_CLOSE(fd);

	addr1.sll_family = AF_PACKET;
	addr1.sll_ifindex = ifr.ifr_ifindex;
	addr2.sll_family = AF_PACKET;

	fzsync_pair.exec_loops = 10000;
	tst_fzsync_pair_init(&fzsync_pair);
}

static void cleanup(void)
{
	tst_fzsync_pair_cleanup(&fzsync_pair);
}

static void do_bind(void)
{
	SAFE_BIND(fd, (struct sockaddr *)&addr1, sizeof(addr1));
	SAFE_BIND(fd, (struct sockaddr *)&addr2, sizeof(addr2));
}

static void *thread_run(void *arg)
{
	while (tst_fzsync_run_b(&fzsync_pair)) {
		tst_fzsync_start_race_b(&fzsync_pair);
		do_bind();
		tst_fzsync_end_race_b(&fzsync_pair);
	}

	return arg;
}

static void run(void)
{
	struct ifreq ifr;

	tst_fzsync_pair_reset(&fzsync_pair, thread_run);
	strcpy(ifr.ifr_name, "lo");

	while (tst_fzsync_run_a(&fzsync_pair)) {
		fd = SAFE_SOCKET(AF_PACKET, SOCK_DGRAM, PF_PACKET);
		ifr.ifr_flags = 0;
		ioctl(fd, SIOCSIFFLAGS, &ifr);
		ifr.ifr_flags = IFF_UP;
		tst_fzsync_start_race_a(&fzsync_pair);
		ioctl(fd, SIOCSIFFLAGS, &ifr);
		tst_fzsync_end_race_a(&fzsync_pair);
		SAFE_CLOSE(fd);
	}

	tst_res(TPASS, "Nothing bad happened (yet)");
}

static struct tst_test test = {
	.test_all = run,
	.setup = setup,
	.cleanup = cleanup,
	.timeout = 600,
	.taint_check = TST_TAINT_W | TST_TAINT_D,
	.needs_kconfigs = (const char *[]) {
		"CONFIG_USER_NS=y",
		"CONFIG_NET_NS=y",
		NULL
	},
	.save_restore = (const char * const[]) {
		"?/proc/sys/user/max_user_namespaces",
		NULL,
	},
	.tags = (const struct tst_tag[]) {
		{"linux-git", "15fe076edea7"},
		{"CVE", "2018-18559"},
		{}
	}
};
