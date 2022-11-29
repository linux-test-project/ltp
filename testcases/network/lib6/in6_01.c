// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines Corp., 2001
 * Copyright (c) 2017 Petr Vorel <pvorel@suse.cz>
 * Copyright (c) Linux Test Project, 2005-2022
 * Author: David L Stevens
 */

/*\
 * [Description]
 *
 * Verify that in6 and sockaddr fields are present.
 */

#include <errno.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include "tst_test.h"
#include "tst_safe_macros.h"

static struct {
	char *addr;
	int ismap;
} maptab[] = {
	{ "2002::1", 0 },
	{ "::ffff:10.0.0.1", 1 },
	{ "::fffe:10.0.0.1", 0 },
	{ "::7fff:10.0.0.1", 0 },
	{ "0:0:0:0:0:0:ffff:a001", 0 },
	{ "0:0:1:0:0:0:ffff:a001", 0 },
};

static struct {
	char *addr;
} sstab[] = {
	{ "2002::1" },
	{ "10.0.0.1" },
	{ "::ffff:10.0.0.1" },
	{ "::1" },
	{ "::" },
};

static void test_in6_addr(void);
static void test_sockaddr_in6(void);
static void test_global_in6_def(void);
static void test_in6_is_addr_v4mapped(void);
static void test_sockaddr_storage(void);

static void (*testfunc[])(void) = { test_in6_addr,
	test_sockaddr_in6, test_global_in6_def,
	test_in6_is_addr_v4mapped, test_sockaddr_storage };

/* struct in6_addr tests */
static void test_in6_addr(void)
{
	uint8_t ui8 = 1;
	struct in6_addr in6;

	in6.s6_addr[0] = ui8;
	tst_res(TINFO, "type of in6.s6_addr[0] is uint8_t");
	if (sizeof(in6.s6_addr) != 16)
		tst_res(TFAIL, "sizeof(in6.s6_addr) != 16");
	else
		tst_res(TPASS, "sizeof(in6.s6_addr) == 16");
}

/* struct sockaddr_in6 tests */
static void test_sockaddr_in6(void)
{
	uint8_t ui8 = 1;
	uint32_t ui16 = 2;
	uint32_t ui32 = 3;
	struct in6_addr in6;
	struct sockaddr_in6 sin6;
	int sd;

	in6.s6_addr[0] = ui8;
	sin6.sin6_family = AF_INET6;
	sin6.sin6_port = ui16;
	sin6.sin6_flowinfo = ui32;
	sin6.sin6_addr = in6;
	sin6.sin6_scope_id = ui32;

	sd = SAFE_SOCKET(AF_INET6, SOCK_STREAM, 0);
	bind(sd, (struct sockaddr *)&sin6, sizeof(sin6));
	SAFE_CLOSE(sd);

	tst_res(TPASS, "all sockaddr_in6 fields present and correct");
}

/* initializers and global in6 definitions tests */
static void test_global_in6_def(void)
{
	struct in6_addr ina6 = IN6ADDR_ANY_INIT;
	struct in6_addr inl6 = IN6ADDR_LOOPBACK_INIT;

	tst_res(TINFO, "IN6ADDR_ANY_INIT present");
	if (memcmp(&ina6, &in6addr_any, sizeof(ina6)) == 0)
		tst_res(TINFO, "in6addr_any present and correct");
	else {
		tst_res(TFAIL, "in6addr_any incorrect value");
		return;
	}

	tst_res(TINFO, "IN6ADDR_LOOPBACK_INIT present");
	if (memcmp(&inl6, &in6addr_loopback, sizeof(inl6)) == 0)
		tst_res(TINFO, "in6addr_loopback present and correct");
	else {
		tst_res(TFAIL, "in6addr_loopback incorrect value");
		return;
	}

	if (inet_pton(AF_INET6, "::1", &inl6) <= 0)
		tst_brk(TBROK | TERRNO, "inet_pton(\"::1\")");

	if (memcmp(&inl6, &in6addr_loopback, sizeof(inl6)) == 0)
		tst_res(TINFO, "in6addr_loopback in network byte order");
	else {
		tst_res(TFAIL, "in6addr_loopback has wrong byte order");
		return;
	}

	tst_res(TPASS, "global in6 definitions tests succeed");
}

/* IN6_IS_ADDR_V4MAPPED tests */
static void test_in6_is_addr_v4mapped(void)
{
	unsigned int i;
	struct in6_addr in6;

	for (i = 0; i < ARRAY_SIZE(maptab); ++i) {
		if (inet_pton(AF_INET6, maptab[i].addr, &in6) <= 0)
			tst_brk(TBROK | TERRNO,
				"\"%s\" is not a valid IPv6 address",
				maptab[i].addr);
		TEST(IN6_IS_ADDR_V4MAPPED(in6.s6_addr));
		if (maptab[i].ismap == TST_RET)
			tst_res(TINFO, "IN6_IS_ADDR_V4MAPPED(\"%s\") %ld",
				maptab[i].addr, TST_RET);
		else {
			tst_res(TFAIL, "IN6_IS_ADDR_V4MAPPED(\"%s\") %ld",
				maptab[i].addr, TST_RET);
			return;
		}
	}

	tst_res(TPASS, "IN6_IS_ADDR_V4MAPPED tests succeed");
}

/* sockaddr_storage tests */
static void test_sockaddr_storage(void)
{
	unsigned int i;
	struct sockaddr_storage ss;

	if (sizeof(ss) <= sizeof(struct sockaddr_in) ||
		sizeof(ss) <= sizeof(struct sockaddr_in6))
		tst_brk(TBROK, "sockaddr_storage too small");

	for (i = 0; i < ARRAY_SIZE(sstab); ++i) {
		struct sockaddr_in *psin = (struct sockaddr_in *)&ss;
		struct sockaddr_in6 *psin6 = (struct sockaddr_in6 *)&ss;
		int rv;
		uint8_t af;

		af = psin->sin_family = AF_INET;
		rv = inet_pton(AF_INET, sstab[i].addr, &psin->sin_addr);
		if (rv == 0) {
			af = psin6->sin6_family = AF_INET6;
			rv = inet_pton(AF_INET6, sstab[i].addr,
				&psin6->sin6_addr);
		}
		if (rv <= 0)
			tst_brk(TBROK,
				"\"%s\" is not a valid address", sstab[i].addr);
		if (ss.ss_family == af)
			tst_res(TINFO, "\"%s\" is AF_INET%s",
				sstab[i].addr, af == AF_INET ? "" : "6");
		else
			tst_res(TFAIL, "\"%s\" ss_family (%d) != AF_INET%s",
				sstab[i].addr, af, af == AF_INET ? "" : "6");
	}

	tst_res(TPASS, "sockaddr_storage tests succeed");
}

static void do_test(unsigned int i)
{
	testfunc[i]();
}

static struct tst_test test = {
	.tcnt = ARRAY_SIZE(testfunc),
	.test = do_test,
};
