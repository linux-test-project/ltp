// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2018 Oracle and/or its affiliates. All Rights Reserved.
 * Copyright (c) Linux Test Project, 2019-2023
 */

/*\
 * [Description]
 *
 * Regression test for the crash caused by over-sized SCTP chunk,
 * fixed by upstream commit 07f2c7ab6f8d ("sctp: verify size of a new
 * chunk in _sctp_make_chunk()").
 */

#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip6.h>
#include <netdb.h>
#include <sys/syscall.h>

#include "tst_test.h"
#include "tst_safe_stdio.h"
#include "tst_checksum.h"
#include "lapi/netinet_in.h"
#include "lapi/socket.h"
#include "lapi/sctp.h"

static int port;
static int sfd, cfd;
static struct sockaddr_in6 rmt, loc;

static uint8_t packet[IP_MAXPACKET];
static int pkt_len;
static char *addr_param;
static int addr_num = 3273;

static void setup_server(void)
{
	const char hmac_algo_path[] = "/proc/sys/net/sctp/cookie_hmac_alg";
	char hmac_algo[CHAR_MAX];
	int hmac_algo_changed = 0;
	int fd;

	/* Disable md5 if fips is enabled. Set it to none */
	if (tst_fips_enabled()) {
		/* Load sctp module */
		if (access(hmac_algo_path, F_OK) < 0) {
			fd = SAFE_SOCKET(PF_INET, SOCK_STREAM, IPPROTO_SCTP);
			SAFE_CLOSE(fd);
		}

		if (!access(hmac_algo_path, F_OK)) {
			SAFE_FILE_SCANF(hmac_algo_path, "%s", hmac_algo);
			SAFE_FILE_PRINTF(hmac_algo_path, "%s", "none");
			hmac_algo_changed = 1;
		}
	}

	loc.sin6_family = AF_INET6;
	loc.sin6_addr = in6addr_loopback;

	sfd = SAFE_SOCKET(AF_INET6, SOCK_STREAM, IPPROTO_SCTP);
	SAFE_BIND(sfd, (struct sockaddr *)&loc, sizeof(loc));

	port = TST_GETSOCKPORT(sfd);
	tst_res(TINFO, "sctp server listen on %d", port);

	SAFE_LISTEN(sfd, 1);

	srand(port);

	if (hmac_algo_changed)
		SAFE_FILE_PRINTF(hmac_algo_path, "%s", hmac_algo);
}

static void update_packet_field(size_t *off, void *buf, size_t buf_len)
{
	memcpy(packet + *off, buf, buf_len);
	*off += buf_len;
}

static void setup_client(void)
{
	struct ip6_hdr ip6;
	const size_t ip6_hdr_len = sizeof(ip6);
	size_t cmn_hdr_off;
	size_t off;
	int i;

	memset(&ip6, 0, sizeof(ip6));
	ip6.ip6_flow = htonl(6 << 28 | 2 << 20);
	ip6.ip6_hops = 64;
	ip6.ip6_nxt = IPPROTO_SCTP;
	ip6.ip6_src.s6_addr[15] = 1;
	ip6.ip6_dst.s6_addr[15] = 1;
	rmt.sin6_family = AF_INET6;
	rmt.sin6_addr = in6addr_loopback;

	/* SCTP common header */
	off = ip6_hdr_len;

	uint16_t src_port = htons(port - 1);
	uint16_t dst_port = htons(port);
	uint32_t vtag = 0;
	uint32_t checksum = 0;

	update_packet_field(&off, &src_port, 2);
	update_packet_field(&off, &dst_port, 2);
	update_packet_field(&off, &vtag, 4);
	update_packet_field(&off, &checksum, 4);
	cmn_hdr_off = off;

	/* SCTP INIT chunk */
	uint16_t chunk_len;

	packet[off++] = 1;
	packet[off++] = 0;
	off += 2; /* chunk length, will be set in the end */

	uint32_t init_tag = rand();
	uint32_t rwnd = htonl(106496);
	uint16_t outs = htons(10);
	uint16_t ins = htons(65535);
	uint32_t init_tsn = rand();

	update_packet_field(&off, &init_tag, 4);
	update_packet_field(&off, &rwnd, 4);
	update_packet_field(&off, &outs, 2);
	update_packet_field(&off, &ins, 2);
	update_packet_field(&off, &init_tsn, 4);

	/* SCTP optional parameter for IPv6 addresses */
	uint16_t param_type = htons(6);
	uint16_t param_len = htons(20);

	/* IPv6(40) + SCTP_COMMON(12) + SCTP_CHUNK(20) + SCTP_OPT(65460)) */
	for (i = 0; i < addr_num; ++i) {
		update_packet_field(&off, &param_type, 2);
		update_packet_field(&off, &param_len, 2);
		packet[off + 15] = 1;
		off += 16;
	}
	pkt_len = off;

	tst_res(TINFO, "set chunk length %zu", pkt_len - cmn_hdr_off);
	chunk_len = htons(pkt_len - cmn_hdr_off);
	memcpy(packet + cmn_hdr_off + 2, &chunk_len, 2);

	/* set checksum for SCTP: common header + INIT chunk */
	uint32_t csum = tst_crc32c(packet + ip6_hdr_len, pkt_len - ip6_hdr_len);

	memcpy(packet + ip6_hdr_len + 8, &csum, 4);

	ip6.ip6_plen = htons(pkt_len - ip6_hdr_len);
	memcpy(packet, &ip6, ip6_hdr_len);

	cfd = SAFE_SOCKET(AF_INET6, SOCK_RAW, IPPROTO_RAW);
}

static const char mtu_path[] = "/sys/class/net/lo/mtu";
static const unsigned int max_mtu = 65535;
static unsigned int mtu;

static void setup(void)
{
	if (tst_parse_int(addr_param, &addr_num, 1, INT_MAX))
		tst_brk(TBROK, "wrong address number '%s'", addr_param);

	/* We don't fragment IPv6 packet here yet, check that MTU is 65535 */
	SAFE_FILE_SCANF(mtu_path, "%d", &mtu);
	if (mtu < max_mtu)
		tst_brk(TCONF, "Test needs that 'lo' MTU has %d", max_mtu);

	setup_server();
	setup_client();
}

static void run(void)
{
	int pid = SAFE_FORK();

	if (!pid) {
		struct sockaddr_in6 addr6;
		socklen_t addr_size = sizeof(addr6);

		if (accept(sfd, (struct sockaddr *)&addr6, &addr_size) < 0)
			tst_brk(TBROK | TERRNO, "accept() failed");
		exit(0);
	}

	SAFE_SENDTO(1, cfd, packet, pkt_len, 0, (struct sockaddr *)&rmt,
		    sizeof(rmt));

	SAFE_KILL(pid, SIGKILL);
	SAFE_WAITPID(pid, NULL, 0);

	tst_res(TPASS, "test doesn't cause crash");
}

static struct tst_test test = {
	.needs_root = 1,
	.setup = setup,
	.forks_child = 1,
	.test_all = run,
	.options = (struct tst_option[]) {
		{"a:", &addr_param, "Number of additional IP address params"},
		{}
	},
	.tags = (const struct tst_tag[]) {
		{"CVE", "2018-5803"},
		{"linux-git", "07f2c7ab6f8d"},
		{}
	}
};
