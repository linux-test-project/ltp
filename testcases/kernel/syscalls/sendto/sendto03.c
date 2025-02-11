// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2019 SUSE LLC <mdoucha@suse.cz>
 */

/*\
 * CVE-2020-14386
 *
 * Check for vulnerability in tpacket_rcv() which allows an unprivileged user
 * to write arbitrary data to a memory area outside the allocated packet
 * buffer.
 *
 * Kernel crash fixed in 5.9
 * acf69c946233 ("net/packet: fix overflow in tpacket_rcv")
 */

#include <stdio.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <net/ethernet.h>

#include "tst_test.h"
#include "tst_net.h"
#include "lapi/if_packet.h"

#define BUFSIZE 1024

static int dst_sock = -1, sock = -1;
static unsigned char buf[BUFSIZE];
static struct sockaddr_ll bind_addr, addr;

static void setup(void)
{
	struct ifreq ifr;

	tst_setup_netns();

	sock = SAFE_SOCKET(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
	strcpy(ifr.ifr_name, "lo");
	ifr.ifr_flags = IFF_UP;
	SAFE_IOCTL(sock, SIOCSIFFLAGS, &ifr);
	SAFE_IOCTL(sock, SIOCGIFINDEX, &ifr);
	SAFE_CLOSE(sock);

	memset(buf, 0x42, BUFSIZE);

	bind_addr.sll_family = AF_PACKET;
	bind_addr.sll_protocol = htons(ETH_P_ALL);
	bind_addr.sll_ifindex = ifr.ifr_ifindex;

	addr.sll_family = AF_PACKET;
	addr.sll_ifindex = ifr.ifr_ifindex;
	addr.sll_halen = ETH_ALEN;
}

/* Test for commit bcc5364bdcfe (cap PACKET_RESERVE to INT_MAX) */
static int check_tiny_frame(void)
{
	unsigned int val = (UINT_MAX - TPACKET2_HDRLEN) + 1;
	struct tpacket_req tpreq;

	tpreq.tp_block_size = SAFE_SYSCONF(_SC_PAGESIZE);
	tpreq.tp_frame_size = TPACKET_ALIGNMENT;
	tpreq.tp_block_nr = 1;
	tpreq.tp_frame_nr = (tpreq.tp_block_size * tpreq.tp_block_nr) /
		tpreq.tp_frame_size;

	dst_sock = SAFE_SOCKET(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
	SAFE_SETSOCKOPT_INT(dst_sock, SOL_PACKET, PACKET_VERSION, TPACKET_V2);
	TEST(setsockopt(dst_sock, SOL_PACKET, PACKET_RESERVE, &val,
		sizeof(val)));

	if (TST_RET == -1 && TST_ERR == EINVAL) {
		SAFE_CLOSE(dst_sock);
		tst_res(TPASS | TTERRNO,
			"setsockopt(PACKET_RESERVE) value is capped");
		return 0;
	}

	if (TST_RET == -1) {
		tst_brk(TBROK | TTERRNO,
			"setsockopt(PACKET_RESERVE): unexpected error");
	}

	if (TST_RET) {
		tst_brk(TBROK | TTERRNO,
			"Invalid setsockopt(PACKET_RESERVE) return value");
	}

	tst_res(TINFO, "setsockopt(PACKET_RESERVE) accepted too large value");
	tst_res(TINFO, "Checking whether this will cause integer overflow...");
	TEST(setsockopt(dst_sock, SOL_PACKET, PACKET_RX_RING, &tpreq,
		sizeof(tpreq)));
	SAFE_CLOSE(dst_sock);

	if (!TST_RET) {
		tst_res(TFAIL, "setsockopt(PACKET_RX_RING) accepted frame "
			"size smaller than packet header");
		return 0;
	}

	if (TST_RET != -1) {
		tst_brk(TBROK | TTERRNO,
			"Invalid setsockopt(PACKET_RX_RING) return value");
	}

	if (TST_ERR != EINVAL) {
		tst_brk(TBROK | TTERRNO,
			"setsockopt(PACKET_RX_RING): unexpeced error");
	}

	tst_res(TPASS | TTERRNO, "setsockopt(PACKET_RX_RING) frame size check "
		"rejects values smaller than packet header");
	/* This test case should not cause kernel taint, skip taint check */
	return 0;
}

/* Test for commit acf69c946233 (drop packet if netoff overflows) */
static int check_vnet_hdr(void)
{
	struct tpacket_req tpreq;
	size_t blocksize = 0x800000, pagesize = SAFE_SYSCONF(_SC_PAGESIZE);

	/* Make sure blocksize is big enough and pagesize aligned */
	if (blocksize % pagesize)
		blocksize += pagesize - (blocksize % pagesize);

	tpreq.tp_block_size = blocksize;
	tpreq.tp_frame_size = 0x11000;
	tpreq.tp_block_nr = 1;
	tpreq.tp_frame_nr = (tpreq.tp_block_size * tpreq.tp_block_nr) /
		tpreq.tp_frame_size;

	dst_sock = SAFE_SOCKET(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
	SAFE_SETSOCKOPT_INT(dst_sock, SOL_PACKET, PACKET_VERSION, TPACKET_V2);
	SAFE_SETSOCKOPT_INT(dst_sock, SOL_PACKET, PACKET_VNET_HDR, 1);
	SAFE_SETSOCKOPT_INT(dst_sock, SOL_PACKET, PACKET_RESERVE, 0xffff - 75);
	TEST(setsockopt(dst_sock, SOL_PACKET, PACKET_RX_RING, &tpreq,
		sizeof(tpreq)));

	if (TST_RET == -1 && TST_ERR == EINVAL) {
		SAFE_CLOSE(dst_sock);
		tst_res(TCONF, "PACKET_VNET_HDR and PACKET_RX_RING not "
			"supported together");
		return 0;
	}

	if (TST_RET == -1) {
		tst_brk(TBROK | TTERRNO,
			"setsockopt(PACKET_RX_RING): unexpected error");
	}

	if (TST_RET) {
		tst_brk(TBROK | TTERRNO,
			"Invalid setsockopt(PACKET_RX_RING) return value");
	}

	SAFE_BIND(dst_sock, (struct sockaddr *)&bind_addr, sizeof(addr));

	sock = SAFE_SOCKET(AF_PACKET, SOCK_RAW, IPPROTO_RAW);
	SAFE_SENDTO(1, sock, buf, BUFSIZE, 0, (struct sockaddr *)&addr,
		sizeof(addr));

	SAFE_CLOSE(sock);
	SAFE_CLOSE(dst_sock);
	return 1; /* Continue to taint check */
}

static int (*testcase_list[])(void) = {check_tiny_frame, check_vnet_hdr};

static void run(unsigned int n)
{
	if (!testcase_list[n]())
		return;

	if (tst_taint_check()) {
		tst_res(TFAIL, "Kernel is vulnerable");
		return;
	}

	tst_res(TPASS, "Nothing bad happened, probably");
}

static void cleanup(void)
{
	if (sock != -1)
		SAFE_CLOSE(sock);

	if (dst_sock != -1)
		SAFE_CLOSE(dst_sock);
}

static struct tst_test test = {
	.test = run,
	.tcnt = ARRAY_SIZE(testcase_list),
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
		{"linux-git", "bcc5364bdcfe"},
		{"linux-git", "acf69c946233"},
		{"CVE", "2020-14386"},
		{}
	}
};
