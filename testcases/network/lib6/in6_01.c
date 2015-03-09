/*
 *
 *   Copyright (c) International Business Machines  Corp., 2001
 *   Author: David L Stevens
 *
 *   This program is free software;  you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY;  without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
 *   the GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program;  if not, write to the Free Software Foundation,
 *   Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */
/*
 *   Description:
 *     Verify that in6 and sockaddr fields are present. Most of these are
 *     "PASS" if they just compile.
 */

#include <stdio.h>
#include <unistd.h>
#include <errno.h>

#include <netinet/in.h>
#include <arpa/inet.h>

#include "test.h"

static struct {
	char *addr;
	int ismap;
} maptab[] = {
	{ "2002::1", 0 },
	{ "::ffff:10.0.0.1", 1 },
	{ "::fffe:10.0.0.1", 0 },
	{ "::7fff:10.0.0.1", 0 },
	{ "0:0:0:0:0:0:ffff:0a001", 0 },
	{ "0:0:1:0:0:0:ffff:0a001", 0 },
};

#define MAPSIZE (sizeof(maptab)/sizeof(maptab[0]))

static struct {
	char *addr;
} sstab[] = {
	{ "2002::1" },
	{ "10.0.0.1" },
	{ "::ffff:10.0.0.1" },
	{ "::1" },
	{ "::" },
};

#define SSSIZE (sizeof(sstab)/sizeof(sstab[0]))

static void setup(void);
static void test_in6_addr(void);
static void test_sockaddr_in6(void);
static void test_global_in6_def(void);
static void test_in6_is_addr_v4mapped(void);
static void test_sockaddr_storage(void);

static void (*testfunc[])(void) = { test_in6_addr,
	test_sockaddr_in6, test_global_in6_def,
	test_in6_is_addr_v4mapped, test_sockaddr_storage };

char *TCID = "in6_01";
int TST_TOTAL = ARRAY_SIZE(testfunc);

int main(int argc, char *argv[])
{
	int lc;
	int i;

	tst_parse_opts(argc, argv, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		tst_count = 0;

		for (i = 0; i < TST_TOTAL; i++)
			(*testfunc[i])();
	}

	tst_exit();
}

static void setup(void)
{
	TEST_PAUSE;
}

/* struct in6_addr tests */
static void test_in6_addr(void)
{
	uint8_t ui8 = 1;
	struct in6_addr in6;

	in6.s6_addr[0] = ui8;
	tst_resm(TINFO, "type of in6.s6_addr[0] is uint8_t");
	if (sizeof(in6.s6_addr) != 16)
		tst_resm(TFAIL, "sizeof(in6.s6_addr) != 16");
	else
		tst_resm(TPASS, "sizeof(in6.s6_addr) == 16");
}

/* struct sockaddr_in6 tests */
static void test_sockaddr_in6(void)
{
	uint8_t ui8 = 1;
	uint32_t ui16 = 2;
	uint32_t ui32 = 3;
	struct in6_addr in6;
	struct sockaddr_in6 sin6;

	in6.s6_addr[0] = ui8;
	sin6.sin6_family = AF_INET6;
	sin6.sin6_port = ui16;
	sin6.sin6_flowinfo = ui32;
	sin6.sin6_addr = in6;
	sin6.sin6_scope_id = ui32;
	tst_resm(TPASS, "all sockaddr_in6 fields present and correct");
}

/* initializers and global in6 definitions tests */
static void test_global_in6_def(void)
{
	struct in6_addr ina6 = IN6ADDR_ANY_INIT;
	struct in6_addr inl6 = IN6ADDR_LOOPBACK_INIT;

	tst_resm(TINFO, "IN6ADDR_ANY_INIT present");
	if (memcmp(&ina6, &in6addr_any, sizeof(ina6)) == 0) {
		tst_resm(TINFO, "in6addr_any present and correct");
	} else {
		tst_resm(TFAIL, "in6addr_any incorrect value");
		return;
	}

	tst_resm(TINFO, "IN6ADDR_LOOPBACK_INIT present");
	if (memcmp(&inl6, &in6addr_loopback, sizeof(inl6)) == 0) {
		tst_resm(TINFO, "in6addr_loopback present and correct");
	} else {
		tst_resm(TFAIL, "in6addr_loopback incorrect value");
		return;
	}
	if (inet_pton(AF_INET6, "::1", &inl6) <= 0)
		tst_brkm(TBROK | TERRNO, NULL, "inet_pton(\"::1\")");
	if (memcmp(&inl6, &in6addr_loopback, sizeof(inl6)) == 0) {
		tst_resm(TINFO, "in6addr_loopback in network byte order");
	} else {
		tst_resm(TFAIL, "in6addr_loopback has wrong byte order");
		return;
	}

	tst_resm(TPASS, "global in6 definitions tests succeed");
}

/* IN6_IS_ADDR_V4MAPPED tests */
static void test_in6_is_addr_v4mapped(void)
{
	unsigned int i;
	struct in6_addr in6;

	for (i = 0; i < MAPSIZE; ++i) {
		if (inet_pton(AF_INET6, maptab[i].addr, &in6) <= 0) {
			tst_brkm(TBROK | TERRNO, NULL,
				"\"%s\" is not a valid IPv6 address",
				maptab[i].addr);
		}
		TEST(IN6_IS_ADDR_V4MAPPED(in6.s6_addr));
		if (TEST_RETURN == maptab[i].ismap) {
			tst_resm(TINFO, "IN6_IS_ADDR_V4MAPPED(\"%s\") %ld",
				maptab[i].addr, TEST_RETURN);
		} else {
			tst_resm(TFAIL, "IN6_IS_ADDR_V4MAPPED(\"%s\") %ld",
				maptab[i].addr, TEST_RETURN);
			return;
		}
	}

	tst_resm(TPASS, "IN6_IS_ADDR_V4MAPPED tests succeed");
}

/* sockaddr_storage tests */
static void test_sockaddr_storage(void)
{
	unsigned int i;
	struct sockaddr_storage ss;

	if (sizeof(ss) <= sizeof(struct sockaddr_in) ||
		sizeof(ss) <= sizeof(struct sockaddr_in6))
		tst_brkm(TBROK, NULL, "sockaddr_storage too small");

	for (i = 0; i < SSSIZE; ++i) {
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
		if (rv <= 0) {
			tst_brkm(TBROK, NULL,
				"\"%s\" is not a valid address", sstab[i].addr);
		}
		if (ss.ss_family == af) {
			tst_resm(TINFO, "\"%s\" is AF_INET%s",
				sstab[i].addr, af == AF_INET ? "" : "6");
		} else {
			tst_resm(TFAIL, "\"%s\" ss_family (%d) != AF_INET%s",
				sstab[i].addr, af, af == AF_INET ? "" : "6");
		}
	}

	tst_resm(TPASS, "sockaddr_storage tests succeed");
}
