// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2021, BELLSOFT. All rights reserved.
 * Copyright (c) 2015 Fujitsu Ltd.
 * Copyright (c) International Business Machines  Corp., 2001
 *
 * Author: David L Stevens
 */

/*\
 * Basic getaddrinfo() tests.
 *
 * The test adds LTP specific addresses and names to /etc/hosts to avoid
 * DNS, hostname setup issues and conflicts with existing configuration.
 */

#include <unistd.h>
#include <errno.h>
#include <stdlib.h>

#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/param.h>

#include "tst_safe_stdio.h"
#include "tst_test.h"
#include "tst_safe_net.h"

#ifndef AI_V4MAPPED
# define AI_V4MAPPED    0x0008	/* IPv4 mapped addresses are acceptable.  */
#endif

static const char *const host_file = "/etc/hosts";
static const char *hostname;
static const char *shortname;
static sa_family_t family;
static int host_file_changed;

static void verify_res(struct addrinfo *res, int sock_type, in_port_t servnum,
		       int (*test_cb)(struct addrinfo *))
{
	sa_family_t sin_family = 0;
	in_port_t sin_port = 0;
	struct addrinfo *p = res;
	int got_tcp = 0;
	int got_udp = 0;
	int ret = 0;

	size_t exp_addrlen = (family == AF_INET) ? sizeof(struct sockaddr_in) :
			     sizeof(struct sockaddr_in6);

	for (; p; p = p->ai_next) {
		ret |= p->ai_family != family;
		ret |= p->ai_addrlen != exp_addrlen;
		ret |= p->ai_addr == 0;
		got_tcp |= p->ai_socktype == SOCK_STREAM;
		got_udp |= p->ai_socktype == SOCK_DGRAM;

		if (p->ai_addr) {

			if (test_cb)
				ret |= test_cb(p);

			if (p->ai_family == AF_INET) {
				struct sockaddr_in *psin;

				psin = (struct sockaddr_in *)p->ai_addr;
				sin_family = psin->sin_family;
				sin_port = psin->sin_port;
			} else {
				struct sockaddr_in6 *psin6;

				psin6 = (struct sockaddr_in6 *)p->ai_addr;
				sin_family = psin6->sin6_family;
				sin_port = psin6->sin6_port;
			}

			ret |= sin_family != family;
			ret |= sin_port != htons(servnum);
		}

		if (ret)
			break;
	}

	if (!sock_type && (!got_tcp || !got_udp)) {
		tst_brk(TFAIL, "socktype 0,%d TCP %d UDP %d",
			htons(sin_port), got_tcp, got_udp);
	}

	if (ret) {
		tst_brk(TFAIL, "family %d alen %d sin family %d port %d",
			p->ai_family, p->ai_addrlen, sin_family,
			htons(sin_port));
	}
}

static void print_test_family(const char *name)
{
	tst_res(TINFO, "test %s: %s", (family == AF_INET) ? "IPv4" : "IPv6",
		name);
}

static void check_addrinfo(int safe, const char *name, const char *host,
			   in_port_t servnum, const char *service,
			   int flags, int type, int proto,
			   int (*test_cb)(struct addrinfo *))
{
	struct addrinfo *res = NULL;
	struct addrinfo hints;

	print_test_family(name);

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = family;
	hints.ai_flags = flags;
	hints.ai_socktype = type;
	hints.ai_protocol = proto;

	if (safe)
		SAFE_GETADDRINFO(host, service, &hints, &res);
	else
		TEST(getaddrinfo(host, service, &hints, &res));

	if (res) {
		verify_res(res, type, servnum, test_cb);
		freeaddrinfo(res);
		tst_res(TPASS, "%s", name);
	}
}

static void check_addrinfo_name(const char *name)
{
	struct addrinfo *p, *res;
	struct addrinfo hints;

	print_test_family(name);

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = family;
	hints.ai_flags = AI_CANONNAME;

	SAFE_GETADDRINFO(shortname, 0, &hints, &res);

	for (p = res; p; p = p->ai_next) {
		if (p->ai_canonname)
			break;
	}
	if (!p)
		tst_brk(TFAIL, "%s: no entries with canonical name set", name);
	else if (strcasecmp(hostname, p->ai_canonname))
		tst_brk(TFAIL, "%s: ai_canonname '%s' doesn't match hostname '%s'",
			name, p->ai_canonname, hostname);

	tst_res(TPASS, "%s: ai_canonname '%s'", name, p->ai_canonname);
	freeaddrinfo(res);
}

static void check_addrinfo_badflags(const char *name)
{
	if (TST_RET == EAI_BADFLAGS) {
		tst_res(TPASS, "%s returns %ld '%s'", name,
			TST_RET, gai_strerror(TST_RET));
	} else if (TST_RET) {
		tst_brk(TFAIL, "%s returns %ld '%s'", name,
			TST_RET, gai_strerror(TST_RET));
	}
}

static int test_loopback(struct addrinfo *p)
{
	/* hostname not set; addr should be loopback */
	if (family == AF_INET) {
		struct sockaddr_in *psin = (struct sockaddr_in *)p->ai_addr;

		return psin->sin_addr.s_addr != htonl(INADDR_LOOPBACK);
	} else {
		struct sockaddr_in6 *psin6 = (struct sockaddr_in6 *)p->ai_addr;

		return memcmp(&psin6->sin6_addr, &in6addr_loopback,
		       sizeof(struct in6_addr)) != 0;
	}
}

static int test_passive(struct addrinfo *p)
{
	if (family == AF_INET) {
		struct sockaddr_in *psin = (struct sockaddr_in *)p->ai_addr;

		return psin->sin_addr.s_addr == 0;
	} else {
		struct sockaddr_in6 *psin6 = (struct sockaddr_in6 *)p->ai_addr;

		return memcmp(&psin6->sin6_addr, &in6addr_any,
			      sizeof(struct in6_addr)) == 0;
	}
}

static int test_passive_no_host(struct addrinfo *p)
{
	if (family == AF_INET) {
		struct sockaddr_in *psin = (struct sockaddr_in *)p->ai_addr;

		return psin->sin_addr.s_addr != 0;
	} else {
		struct sockaddr_in6 *psin6 = (struct sockaddr_in6 *)p->ai_addr;

		return memcmp(&psin6->sin6_addr, &in6addr_any,
			      sizeof(struct in6_addr));
	}
}

static void gaiv(void)
{
	check_addrinfo(1, "basic lookup", hostname, 0, NULL, 0, 0, 0, NULL);
	check_addrinfo_name("canonical name");

	/*
	 * These are hard-coded for echo/7 to avoid using getservbyname(),
	 * since it isn't thread-safe and these tests may be re-used
	 * multithreaded. Sigh.
	 */
	check_addrinfo(1, "host+service", hostname, 7, "echo", 0, 0, 0, NULL);

	check_addrinfo(1, "host+service, AI_PASSIVE", hostname, 9462, "9462",
		       AI_PASSIVE, SOCK_STREAM, 0, test_passive);

	check_addrinfo(0, "host+service, AI_NUMERICHOST", hostname, 7, "echo",
		       AI_NUMERICHOST, SOCK_STREAM, 0, NULL);
	if (TST_RET != EAI_NONAME)
		tst_brk(TFAIL, "AI_NUMERICHOST: ret %ld exp %d (EAI_NONAME)",
			TST_RET, EAI_NONAME);
	tst_res(TPASS, "AI_NUMERICHOST: expected %ld (EAI_NONAME)", TST_RET);

	check_addrinfo(1, "0+service, AI_PASSIVE", NULL, 9462, "9462",
		       AI_PASSIVE, SOCK_STREAM, 0, test_passive_no_host);

	check_addrinfo(0, "0+service", NULL, 9462, "9462",
		       0, SOCK_STREAM, 0, test_loopback);
	check_addrinfo_badflags("0+service ('', '9462')");

#ifdef AI_NUMERICSERV
	check_addrinfo(0, "host+service, AI_NUMERICSERV", hostname, 7, "echo",
		       AI_NUMERICSERV, 0, 0, NULL);
	if (TST_RET != EAI_NONAME)
		tst_brk(TFAIL, "AI_NUMERICSERV: returns %ld '%s', expected %d (EAI_NONAME)",
			TST_RET, gai_strerror(TST_RET), EAI_NONAME);
	tst_res(TPASS, "AI_NUMERICSERV: returns %ld (EAI_NONAME)", TST_RET);
#else
	tst_res(TCONF, "AI_NUMERICSERV: flag not implemented");
#endif

	check_addrinfo(0, "SOCK_STREAM/IPPROTO_UDP", NULL, 0, NULL, 0,
		       SOCK_STREAM, IPPROTO_UDP, NULL);
	if (!TST_RET)
		tst_brk(TFAIL, "SOCK_STREAM/IPPROTO_UDP: unexpected pass");
	tst_res(TPASS, "SOCK_STREAM/IPPROTO_UDP: failed as expected");

	check_addrinfo(0, "socktype 0,513", NULL, 513, "513", 0, 0, 0, NULL);
	check_addrinfo_badflags("socktype 0,513");

	check_addrinfo(1, "AI_V4MAPPED", NULL, 513, "513",
		       AI_V4MAPPED, 0, 0, NULL);
}

static struct tcase {
	sa_family_t family;
	const char *const addr;
	const char *const name;
	const char *const alias;
} tcases[] = {
	{ AF_INET, "127.0.127.1", "getaddrinfo01.ltp", "getaddrinfo01-ipv4" },
	{ AF_INET6, "::127", "getaddrinfo01.ipv6.ltp", "getaddrinfo01-ipv6" }
};

static void setup(void)
{
	unsigned int i;
	int fd;

	if (access(host_file, W_OK))
		tst_brk(TCONF | TERRNO, "%s file not available", host_file);

	SAFE_CP(host_file, "hosts");

	host_file_changed = 1;
	fd = SAFE_OPEN(host_file, O_WRONLY|O_APPEND);

	for (i = 0; i < ARRAY_SIZE(tcases); ++i) {
		char *entry;

		SAFE_ASPRINTF(&entry, "%s %s %s\n",
			      tcases[i].addr, tcases[i].name, tcases[i].alias);
		SAFE_WRITE(SAFE_WRITE_ANY, fd, entry, strlen(entry));
		free(entry);
	}
	SAFE_CLOSE(fd);
}

static void cleanup(void)
{
	if (host_file_changed)
		SAFE_CP("hosts", host_file);
}

static void do_test(unsigned int i)
{
	family = tcases[i].family;
	hostname = tcases[i].name;
	shortname = tcases[i].alias;
	gaiv();
}

static struct tst_test test = {
	.needs_root = 1,
	.needs_tmpdir = 1,
	.setup = setup,
	.cleanup = cleanup,
	.tcnt = ARRAY_SIZE(tcases),
	.test = do_test,
};
