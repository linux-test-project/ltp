// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2023 FUJITSU LIMITED. All rights reserved.
 * Copyright (c) International Business Machines  Corp., 2001
 * Author: David L Stevens
 */

/*\
 * Basic test for ICMP6_FILTER.
 *
 * For ICMP6_FILTER usage, refer to: https://man.openbsd.org/icmp6.
 *
 * Because of the extra functionality of ICMPv6 in comparison to ICMPv4, a
 * larger number of messages may be potentially received on an ICMPv6 socket.
 * Input filters may therefore be used to restrict input to a subset of the
 * incoming ICMPv6 messages so only interesting messages are returned by the
 * :man2:`recv` family of calls to an application.

 * The icmp6_filter structure may be used to refine the input message set
 * according to the ICMPv6 type. By default, all messages types are allowed
 * on newly created raw ICMPv6 sockets. The following macros may be used to
 * refine the input set, thus being tested:
 *
 * void ICMP6_FILTER_SETPASSALL(struct icmp6_filter *filterp)
 * &ndash; Allow all incoming messages. filterp is modified to allow all message types.
 *
 * void ICMP6_FILTER_SETBLOCKALL(struct icmp6_filter *filterp)
 * &ndash; Ignore all incoming messages. filterp is modified to ignore all message types.
 *
 * void ICMP6_FILTER_SETPASS(int, struct icmp6_filter *filterp)
 * &ndash; Allow ICMPv6 messages with the given type. filterp is modified to allow such
 * messages.
 *
 * void ICMP6_FILTER_SETBLOCK(int, struct icmp6_filter *filterp)
 * &ndash; Ignore ICMPv6 messages with the given type. filterp is modified to ignore
 * such messages.
 *
 * int ICMP6_FILTER_WILLPASS(int, const struct icmp6_filter *filterp)
 * &ndash; Determine if the given filter will allow an ICMPv6 message of the given type.
 *
 * int ICMP6_FILTER_WILLBLOCK(int, const struct icmp6_filter *)
 * &ndash; Determine if the given filter will ignore an ICMPv6 message of the given type.
 *
 * The :man2:`getsockopt` and :man2:`setsockopt` calls may be used to obtain and
 * install the filter on ICMPv6 sockets at option level ``IPPROTO_ICMPV6`` and
 * name ``ICMP6_FILTER`` with a pointer to the icmp6_filter structure as the
 * option value.
 */

#include <netinet/icmp6.h>
#include "tst_test.h"

static int sall = -1, sf = -1;

enum filter_macro {
	FILTER_SETPASS,
	FILTER_SETBLOCK,
	FILTER_PASSALL,
	FILTER_BLOCKALL,
	FILTER_WILLBLOCK,
	FILTER_WILLPASS
};

#define DESC(x, y, z) .tname = "ICMP6_" #x ", send type: " #y ", filter type: " \
	#z, .send_type = y, .filter_type = z, .test_macro = x

static struct tcase {
	char *tname;
	unsigned char send_type;
	unsigned char filter_type;
	enum filter_macro test_macro;
	int pass_packet;
} tcases[] = {
	{DESC(FILTER_SETPASS, 20, 20), .pass_packet = 1},
	{DESC(FILTER_SETPASS, 20, 21)},
	{DESC(FILTER_SETBLOCK, 20, 20)},
	{DESC(FILTER_SETBLOCK, 20, 21), .pass_packet = 1},
	{DESC(FILTER_PASSALL, 20, 20), .pass_packet = 1},
	{DESC(FILTER_PASSALL, 21, 0), .pass_packet = 1},
	{DESC(FILTER_BLOCKALL, 20, 0)},
	{DESC(FILTER_BLOCKALL, 21, 0)},
	{DESC(FILTER_WILLBLOCK, 20, 21)},
	{DESC(FILTER_WILLBLOCK, 20, 20), .pass_packet = 1},
	{DESC(FILTER_WILLPASS, 20, 21)},
	{DESC(FILTER_WILLPASS, 22, 22), .pass_packet = 1},
};

static void ic6_send(unsigned char type)
{
	struct sockaddr_in6 sin6;
	struct icmp6_hdr ic6;
	int s;

	s = SAFE_SOCKET(AF_INET6, SOCK_RAW, IPPROTO_ICMPV6);

	memset(&ic6, 0, sizeof(ic6));
	ic6.icmp6_type = type;
	ic6.icmp6_data32[0] = htonl(getpid());

	memset(&sin6, 0, sizeof(sin6));
	sin6.sin6_family = AF_INET6;
	sin6.sin6_addr = in6addr_loopback;
	SAFE_SENDTO(0, s, &ic6, sizeof(ic6), 0, (struct sockaddr *)&sin6, sizeof(sin6));
}

static int ic6_recv(void)
{
	fd_set readfds, readfds_saved;
	struct timeval tv;
	int maxfd, nfds, gotall, gotone;
	unsigned char rbuf[2048];

	tv.tv_sec = 0;
	tv.tv_usec = 250000;

	FD_ZERO(&readfds_saved);
	FD_SET(sall, &readfds_saved);
	FD_SET(sf, &readfds_saved);
	maxfd = MAX(sall, sf);

	memcpy(&readfds, &readfds_saved, sizeof(readfds));

	gotall = gotone = 0;

	while (!gotall || !gotone) {
		struct icmp6_hdr *pic6 = (struct icmp6_hdr *)rbuf;

		nfds = select(maxfd + 1, &readfds, 0, 0, &tv);
		if (nfds == 0)
			break;

		if (nfds < 0) {
			if (errno == EINTR)
				continue;
			tst_brk(TBROK | TERRNO, "select failed");
		}

		if (FD_ISSET(sall, &readfds)) {
			SAFE_RECV(0, sall, rbuf, sizeof(rbuf), 0);
			if (htonl(pic6->icmp6_data32[0]) == (uint32_t)getpid())
				gotall = 1;
		}

		if (FD_ISSET(sf, &readfds)) {
			SAFE_RECV(0, sf, rbuf, sizeof(rbuf), 0);
			if (htonl(pic6->icmp6_data32[0]) == (uint32_t)getpid())
				gotone = 1;
		}
		memcpy(&readfds, &readfds_saved, sizeof(readfds));
	}

	if (!gotall) {
		tst_res(TFAIL, "recv all time out");
		return -1;
	}

	if (gotone)
		return 1;

	return 0;
}

static void verify_icmp6_filter(unsigned int n)
{
	struct tcase *tc = &tcases[n];
	struct icmp6_filter i6f;
	int rc;

	tst_res(TINFO, "Testing %s", tc->tname);

	switch (tc->test_macro) {
	case FILTER_SETPASS:
		ICMP6_FILTER_SETBLOCKALL(&i6f);
		ICMP6_FILTER_SETPASS(tc->filter_type, &i6f);
		break;
	case FILTER_PASSALL:
		ICMP6_FILTER_SETPASSALL(&i6f);
		break;
	case FILTER_SETBLOCK:
		ICMP6_FILTER_SETPASSALL(&i6f);
		ICMP6_FILTER_SETBLOCK(tc->filter_type, &i6f);
		break;
	case FILTER_BLOCKALL:
		ICMP6_FILTER_SETBLOCKALL(&i6f);
		break;
	case FILTER_WILLBLOCK:
		ICMP6_FILTER_SETPASSALL(&i6f);
		ICMP6_FILTER_SETBLOCK(tc->filter_type, &i6f);
		rc = ICMP6_FILTER_WILLBLOCK(tc->send_type, &i6f);
		TST_EXP_EXPR(rc == tc->pass_packet, "%d (%d)", tc->pass_packet, rc);
		return;
	case FILTER_WILLPASS:
		ICMP6_FILTER_SETBLOCKALL(&i6f);
		ICMP6_FILTER_SETPASS(tc->filter_type, &i6f);
		rc = ICMP6_FILTER_WILLPASS(tc->send_type, &i6f);
		TST_EXP_EXPR(rc == tc->pass_packet, "%d (%d)", tc->pass_packet, rc);
		return;
	default:
		tst_brk(TBROK, "unknown test type %d", tc->filter_type);
		break;
	}

	SAFE_SETSOCKOPT(sf, IPPROTO_ICMPV6, ICMP6_FILTER, &i6f, sizeof(i6f));
	ic6_send(tc->send_type);

	rc = ic6_recv();
	if (rc < 0)
		return;

	TST_EXP_EXPR(rc == tc->pass_packet, "%s packet type %d",
				 rc ? "pass" : "block", tc->send_type);
}

static void setup(void)
{
	struct icmp6_filter i6f;

	sall = SAFE_SOCKET(PF_INET6, SOCK_RAW, IPPROTO_ICMPV6);
	ICMP6_FILTER_SETPASSALL(&i6f);
	SAFE_SETSOCKOPT(sall, IPPROTO_ICMPV6, ICMP6_FILTER, &i6f, sizeof(i6f));

	sf = SAFE_SOCKET(PF_INET6, SOCK_RAW, IPPROTO_ICMPV6);
}

static void cleanup(void)
{
	if (sall > -1)
		SAFE_CLOSE(sall);

	if (sf > -1)
		SAFE_CLOSE(sf);
}

static struct tst_test test = {
	.needs_root = 1,
	.setup = setup,
	.cleanup = cleanup,
	.test = verify_icmp6_filter,
	.tcnt = ARRAY_SIZE(tcases)
};
