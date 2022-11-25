// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2020 SUSE LLC <mdoucha@suse.cz>
 *
 * CVE-2017-17712
 *
 * Test for race condition vulnerability in sendmsg() on SOCK_RAW sockets.
 * Changing the value of IP_HDRINCL socket option in parallel with sendmsg()
 * call may lead to uninitialized stack pointer usage, allowing arbitrary code
 * execution or privilege escalation. Fixed in:
 *
 *  commit 8f659a03a0ba9289b9aeb9b4470e6fb263d6f483
 *  Author: Mohamed Ghannam <simo.ghannam@gmail.com>
 *  Date:   Sun Dec 10 03:50:58 2017 +0000
 *
 *  net: ipv4: fix for a race condition in raw_sendmsg
 */
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "tst_test.h"
#include "tst_fuzzy_sync.h"

#define IOVEC_COUNT 4
#define PACKET_SIZE 100

static int sockfd = -1;
static struct msghdr msg;
/* addr must be full of zeroes to trigger the kernel bug */
static struct sockaddr_in addr;
static struct iovec iov[IOVEC_COUNT];
static unsigned char buf[PACKET_SIZE];
static struct tst_fzsync_pair fzsync_pair;

static void setup(void)
{
	int i;

	tst_setup_netns();

	sockfd = SAFE_SOCKET(AF_INET, SOCK_RAW, IPPROTO_ICMP);

	memset(buf, 0xcc, PACKET_SIZE);

	for (i = 0; i < IOVEC_COUNT; i++) {
		iov[i].iov_base = buf;
		iov[i].iov_len = PACKET_SIZE;
	}

	msg.msg_name = &addr;
	msg.msg_namelen = sizeof(addr);
	msg.msg_iov = iov;
	msg.msg_iovlen = IOVEC_COUNT;

	fzsync_pair.exec_loops = 100000;
	tst_fzsync_pair_init(&fzsync_pair);
}

static void cleanup(void)
{
	if (sockfd > 0)
		SAFE_CLOSE(sockfd);
	tst_fzsync_pair_cleanup(&fzsync_pair);
}

static void *thread_run(void *arg)
{
	int val = 0;

	while (tst_fzsync_run_b(&fzsync_pair)) {
		tst_fzsync_start_race_b(&fzsync_pair);
		setsockopt(sockfd, SOL_IP, IP_HDRINCL, &val, sizeof(val));
		tst_fzsync_end_race_b(&fzsync_pair);
	}

	return arg;
}

static void run(void)
{
	int hdrincl = 1;

	tst_fzsync_pair_reset(&fzsync_pair, thread_run);

	while (tst_fzsync_run_a(&fzsync_pair)) {
		SAFE_SETSOCKOPT_INT(sockfd, SOL_IP, IP_HDRINCL, hdrincl);

		tst_fzsync_start_race_a(&fzsync_pair);
		sendmsg(sockfd, &msg, 0);
		tst_fzsync_end_race_a(&fzsync_pair);

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
	.max_runtime = 150,
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
		{"linux-git", "8f659a03a0ba"},
		{"CVE", "2017-17712"},
		{}
	}
};
