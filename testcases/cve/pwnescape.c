// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2026 Linux Test Project
 */

/*\
 * Regression test for commit 6785011f8b16 ("ipv4: validate
 * checksum_start before completing checksum").
 *
 * A frame written to a TAP device can carry a virtio_net_hdr with
 * VIRTIO_NET_HDR_F_NEEDS_CSUM, asking the stack to complete a partial
 * checksum later. If csum_start points into the IPv4 header instead of
 * at the transport header, skb_checksum_help() completes the checksum
 * and stores the result at csum_start + csum_offset, overwriting the
 * first two bytes of the IPv4 header that were already parsed. When
 * such a packet needs to be fragmented, ip_do_fragment() then reads
 * the corrupted version/IHL byte and copies an attacker-chosen number
 * of "header" bytes, which can read past skb_headlen() and disclose
 * adjacent kernel memory in the IPv4 options of the emitted fragments.
 *
 * The test loads the tun module when run as root and runs inside a
 * private network namespace so that it needs no other privileges.
 *
 * The frames carry two VID 0 VLAN tags because the TAP driver rejects
 * checksum metadata starting less than ``sizeof(struct iphdr)`` bytes into
 * the frame. Without the tags the IPv4 header would start at frame offset
 * 14, so ``csum_start`` could not point at it. With the tags it starts at
 * offset 22.
 *
 * [Algorithm]
 *
 * - create a TAP device with MAC 02:00:00:00:00:01, address
 *   10.0.0.1/24 and MTU 200 inside a fresh network namespace
 * - add a permanent neighbour entry for 10.0.0.3 on the TAP device
 * - enable IPv4 forwarding in the network namespace
 * - open an AF_PACKET (:manpage:`packet(7)`) socket bound to the TAP
 *   device
 * - send a 320 byte control IPv4 datagram behind two VID 0 VLAN tags
 *   with checksum metadata at the transport header (frame offset 42)
 *   and verify its fragments are forwarded with an intact header
 * - send the test datagram with csum_start on the IPv4 header (frame
 *   offset 22) and a payload word chosen so that completing the bogus
 *   checksum writes 0x4f (version 4, IHL 15) at the start of the IPv4
 *   header, then check whether the emitted fragments carry the
 *   corrupted header
 */

#include <stddef.h>

#include <net/if.h>
#include <net/if_arp.h>
#include <linux/if_tun.h>
#include <linux/virtio_net.h>

#include "tst_test.h"
#include "tst_module.h"
#include "tst_netdevice.h"
#include "tst_timer.h"
#include "lapi/if_ether.h"
#include "lapi/if_packet.h"
#include "lapi/ioctl.h"

#define TAP_NAME	"pwnescape"
#define TAP_MTU		200
#define PAYLOAD_LEN	300

#define IP_PROTO	253
#define TTL		64

#define CTRL_ID		0x2000
#define TEST_ID		0x3000
#define GOOD_IHL	5
#define BAD_FIRST_BYTE	0x4f	/* IPv4, IHL 15 */

struct vlan_tag {
	uint16_t tci;
	uint16_t proto;
} __attribute__((packed));

struct pkt_frame {
	struct ethhdr eth;
	struct vlan_tag vlan[2];
	struct iphdr ip;
	uint16_t csum_adj;
	uint8_t payload[PAYLOAD_LEN - sizeof(uint16_t)];
} __attribute__((packed));

static int tap_fd = -1;
static int cap_fd = -1;

static struct pkt_frame ctrl_frame;
static struct pkt_frame test_frame;

static const uint8_t tap_mac[ETH_ALEN] = {0x02, 0x00, 0x00, 0x00, 0x00, 0x01};
static const uint8_t src_mac[ETH_ALEN] = {0x02, 0x00, 0x00, 0x00, 0x00, 0x02};
static const uint8_t nbr_mac[ETH_ALEN] = {0x02, 0x00, 0x00, 0x00, 0x00, 0x03};

static uint16_t ip_checksum(const void *buf, size_t len)
{
	const uint16_t *w = buf;
	uint32_t s = 0;

	for (size_t i = 0; i < len / 2; i++)
		s += w[i];

	s = (s & 0xffff) + (s >> 16);
	s = (s & 0xffff) + (s >> 16);

	return (uint16_t)~s;
}

static void build_frame(struct pkt_frame *fr, uint16_t id, int corrupt_iph)
{
	memset(fr, 0, sizeof(*fr));

	memcpy(fr->eth.h_dest, tap_mac, ETH_ALEN);
	memcpy(fr->eth.h_source, src_mac, ETH_ALEN);
	fr->eth.h_proto = htons(ETH_P_8021Q);
	fr->vlan[0].proto = htons(ETH_P_8021Q);
	fr->vlan[1].proto = htons(ETH_P_IP);

	fr->ip.version = 4;
	fr->ip.ihl = GOOD_IHL;
	fr->ip.tot_len = htons(sizeof(fr->ip) + PAYLOAD_LEN);
	fr->ip.id = htons(id);
	fr->ip.ttl = TTL;
	fr->ip.protocol = IP_PROTO;
	fr->ip.saddr = inet_addr("10.0.0.2");
	fr->ip.daddr = inet_addr("10.0.0.3");
	fr->ip.check = ip_checksum(&fr->ip, sizeof(fr->ip));

	/* a valid IPv4 header always sums to 0xffff (-0 in one's complement)
	 * both before and after the forwarding TTL decrement, so with a zero
	 * payload the folded checksum over the whole IPv4 packet is simply
	 * ~csum_adj
	 */
	if (corrupt_iph)
		fr->csum_adj = htons(~(uint16_t)(BAD_FIRST_BYTE << 8));
}

/* write a frame to the TAP device with a virtio_net_hdr that asks
 * for checksum completion at the given frame-relative csum_start
 */
static void send_datagram(struct pkt_frame *fr, uint16_t csum_start)
{
	struct virtio_net_hdr vnet_hdr = {
		.flags = VIRTIO_NET_HDR_F_NEEDS_CSUM,
		.csum_start = csum_start,
		.csum_offset = 0,
	};
	struct iovec iov[] = {
		{.iov_base = &vnet_hdr, .iov_len = sizeof(vnet_hdr)},
		{.iov_base = fr, .iov_len = sizeof(*fr)},
	};

	SAFE_WRITEV(SAFE_WRITE_ALL, tap_fd, iov, ARRAY_SIZE(iov));
}

static void drain_capture(void)
{
	uint8_t buf[256];
	ssize_t n;

	do {
		n = recv(cap_fd, buf, sizeof(buf), MSG_DONTWAIT);
	} while (n > 0);

	if (n < 0 && errno != EAGAIN && errno != EWOULDBLOCK)
		tst_brk(TBROK | TERRNO, "recv() failed");
}

/* wait up to two seconds for a fragmented packet with the given IP id;
 * store its IPv4 IHL in *ihl and return 1
 */
static int capture_fragment(uint16_t id, unsigned int *ihl)
{
	struct pollfd pfd = {.fd = cap_fd, .events = POLLIN};

	tst_timer_start(CLOCK_MONOTONIC);

	while (!tst_timer_expired_ms(2000)) {
		struct {
			struct ethhdr eth;
			struct iphdr ip;
		} __attribute__((packed)) hdr;
		ssize_t n;

		if (SAFE_POLL(&pfd, 1, 100) <= 0)
			continue;

		n = SAFE_RECV(0, cap_fd, &hdr, sizeof(hdr), 0);
		if (n < (ssize_t)sizeof(hdr))
			continue;

		/* a forwarded fragment has MF set or a non-zero fragment
		 * offset; the injected frame itself never does, which
		 * distinguishes the forwarded output from its receive-side
		 * copy on the same device
		 */
		if (hdr.eth.h_proto == htons(ETH_P_IP) &&
		    ntohs(hdr.ip.id) == id &&
		    ntohs(hdr.ip.frag_off) & (IP_MF | IP_OFFMASK)) {
			*ihl = hdr.ip.ihl;
			return 1;
		}
	}

	return 0;
}

static void run(void)
{
	unsigned int ihl;

	drain_capture();

	send_datagram(&ctrl_frame, offsetof(struct pkt_frame, csum_adj));
	if (!capture_fragment(CTRL_ID, &ihl))
		tst_brk(TBROK, "control datagram was not forwarded back");
	if (ihl != GOOD_IHL)
		tst_brk(TBROK, "control datagram IHL corrupted (%u)", ihl);

	send_datagram(&test_frame, offsetof(struct pkt_frame, ip));
	if (!capture_fragment(TEST_ID, &ihl)) {
		tst_res(TPASS,
			"kernel dropped the datagram with checksum metadata inside the IPv4 header");
		return;
	}

	if (ihl != GOOD_IHL) {
		tst_res(TFAIL, "IPv4 header IHL corrupted by checksum completion (%u)",
			ihl);
		return;
	}

	tst_res(TPASS, "IPv4 header not corrupted by checksum completion");
}

static void setup(void)
{
	struct ifreq ifr;
	struct sockaddr_ll sll;
	struct arpreq req;

	if (!geteuid())
		tst_modprobe("tun", NULL);

	tst_setup_netns();

	/* the tun socket must be opened in the test's own network
	 * namespace: TUNSETIFF checks for CAP_NET_ADMIN in the user
	 * namespace that owns the netns the file was opened in
	 */
	tap_fd = open("/dev/net/tun", O_RDWR);
	if (tap_fd == -1) {
		if (errno == ENOENT || errno == ENODEV)
			tst_brk(TCONF | TERRNO, "TUN support is missing");

		tst_brk(TBROK | TERRNO, "open /dev/net/tun failed");
	}

	memset(&ifr, 0, sizeof(ifr));
	strncpy(ifr.ifr_name, TAP_NAME, IFNAMSIZ - 1);
	ifr.ifr_flags = IFF_TAP | IFF_NO_PI | IFF_VNET_HDR;
	SAFE_IOCTL(tap_fd, TUNSETIFF, &ifr);

	NETDEV_SET_HWADDR(TAP_NAME, tap_mac, sizeof(tap_mac));
	NETDEV_SET_STATE(TAP_NAME, 1);
	NETDEV_ADD_ADDRESS_INET(TAP_NAME, inet_addr("10.0.0.1"), 24, 0);

	cap_fd = SAFE_SOCKET(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));

	memset(&ifr, 0, sizeof(ifr));
	strncpy(ifr.ifr_name, TAP_NAME, IFNAMSIZ - 1);
	ifr.ifr_mtu = TAP_MTU;
	SAFE_IOCTL(cap_fd, SIOCSIFMTU, &ifr);

	memset(&req, 0, sizeof(req));
	tst_init_sockaddr_inet((struct sockaddr_in *)&req.arp_pa, "10.0.0.3", 0);
	req.arp_ha.sa_family = ARPHRD_ETHER;
	memcpy(req.arp_ha.sa_data, nbr_mac, sizeof(nbr_mac));
	req.arp_flags = ATF_COM | ATF_PERM;
	strncpy(req.arp_dev, TAP_NAME, IFNAMSIZ - 1);
	SAFE_IOCTL(cap_fd, SIOCSARP, &req);

	SAFE_FILE_PRINTF("/proc/sys/net/ipv4/ip_forward", "1");

	memset(&sll, 0, sizeof(sll));
	sll.sll_family = AF_PACKET;
	sll.sll_protocol = htons(ETH_P_ALL);
	sll.sll_ifindex = NETDEV_INDEX_BY_NAME(TAP_NAME);
	SAFE_BIND(cap_fd, (struct sockaddr *)&sll, sizeof(sll));

	build_frame(&ctrl_frame, CTRL_ID, 0);
	build_frame(&test_frame, TEST_ID, 1);
}

static void cleanup(void)
{
	if (cap_fd != -1)
		SAFE_CLOSE(cap_fd);

	if (tap_fd != -1)
		SAFE_CLOSE(tap_fd);
}

static struct tst_test test = {
	.test_all = run,
	.setup = setup,
	.cleanup = cleanup,
	.needs_kconfigs = (const char *[]) {
		"CONFIG_TUN",
		"CONFIG_USER_NS=y",
		"CONFIG_NET_NS=y",
		"CONFIG_SYSCTL=y",
		NULL
	},
	.save_restore = (const struct tst_path_val[]) {
		{PATH_USER_MAX_USER_NAMESPACES, "1024", TST_SR_SKIP},
		{}
	},
	.tags = (const struct tst_tag[]) {
		{"linux-git", "6785011f8b16"},
		{"CVE", "2026-99999"},
		{}
	},
};
