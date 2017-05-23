/*
 * Copyright (c) 2015 Fujitsu Ltd.
 * Copyright (c) International Business Machines  Corp., 2001
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Author: David L Stevens
 */

#include <unistd.h>
#include <errno.h>

#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/param.h>

#include "test.h"

#ifndef AI_V4MAPPED
#define AI_V4MAPPED    0x0008	/* IPv4 mapped addresses are acceptable.  */
#endif

static void setup(void);
static void gaiv4(void);
static void gaiv6(void);

char *TCID = "getaddrinfo_01";
int TST_TOTAL = 22;

int main(int argc, char *argv[])
{
	int lc;

	tst_parse_opts(argc, argv, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); ++lc) {
		tst_count = 0;

		gaiv4();
		gaiv6();
	}

	tst_exit();
}

static void setup(void)
{
	TEST_PAUSE;
}

/* getaddrinfo tests (v4) */
static void gaiv4(void)
{
	struct addrinfo *aires, hints, *pai;
	char hostname[MAXHOSTNAMELEN + 1];
	char shortname[MAXHOSTNAMELEN + 1];
	char service[NI_MAXSERV + 1];
	int servnum;
	char *p;

	if (gethostname(hostname, sizeof(hostname)) < 0)
		tst_brkm(TBROK | TERRNO, NULL, "gethostname failed");
	strncpy(shortname, hostname, MAXHOSTNAMELEN);
	shortname[MAXHOSTNAMELEN] = '\0';
	p = strchr(shortname, '.');
	if (p)
		*p = '\0';

	/* test 1, IPv4 basic lookup */
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	TEST(getaddrinfo(hostname, 0, &hints, &aires));
	if (!TEST_RETURN) {
		struct sockaddr_in *psin = 0;
		int err = 0;

		for (pai = aires; pai; pai = pai->ai_next) {
			err |= pai->ai_family != AF_INET;
			err |= pai->ai_addrlen != sizeof(struct sockaddr_in);
			err |= pai->ai_addr == 0;
			psin = (struct sockaddr_in *)pai->ai_addr;
			if (pai->ai_addr) {
				err |= psin->sin_family != AF_INET;
				err |= psin->sin_port != 0;
			}
			if (err)
				break;
		}
		if (err) {
			tst_resm(TFAIL, "getaddrinfo IPv4 basic lookup: "
				 "fam %d alen %d addr 0x%p addr/fam %d "
				 "addr/port %d H[%d]",
				 pai->ai_family, pai->ai_addrlen, psin,
				 psin ? psin->sin_family : 0,
				 psin ? psin->sin_port : 0,
				 psin ? htons(psin->sin_port) : 0);
			freeaddrinfo(aires);
			return;
		}
		tst_resm(TPASS, "getaddrinfo IPv4 basic lookup");
		freeaddrinfo(aires);
	} else {
		tst_resm(TFAIL, "getaddrinfo IPv4 basic "
			 "lookup (\"%s\") returns %ld (\"%s\")", hostname,
			 TEST_RETURN, gai_strerror(TEST_RETURN));
		return;
	}

	/* test 2, IPv4 canonical name */
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_flags = AI_CANONNAME;
	TEST(getaddrinfo(shortname, 0, &hints, &aires));
	if (!TEST_RETURN) {
		for (pai = aires; pai; pai = pai->ai_next)
			if (pai->ai_canonname)
				break;
		if (!pai) {
			tst_resm(TFAIL, "getaddrinfo IPv4 canonical name: no "
				 "entries with canonical name set");
			freeaddrinfo(aires);
			return;
		} else if (strcasecmp(hostname, pai->ai_canonname)) {
			tst_resm(TFAIL, "getaddrinfo IPv4 canonical name "
				 "(\"%s\") doesn't match hostname (\"%s\")",
				 pai->ai_canonname, hostname);
			freeaddrinfo(aires);
			return;
		}
		tst_resm(TPASS, "getaddrinfo IPv4 canonical name");
		freeaddrinfo(aires);
	} else {
		tst_resm(TFAIL, "getaddrinfo IPv4 "
			 "canonical name (\"%s\") returns %ld (\"%s\")",
			 shortname, TEST_RETURN, gai_strerror(TEST_RETURN));
		return;
	}

	/* test 3, IPv4 host+service name */
	memset(&hints, 0, sizeof(hints));
	/*
	 * These are hard-coded for echo/7 to avoid using getservbyname(),
	 * since it isn't thread-safe and these tests may be re-used
	 * multithreaded. Sigh.
	 */
	strcpy(service, "echo");
	servnum = 7;
	hints.ai_family = AF_INET;
	TEST(getaddrinfo(hostname, service, &hints, &aires));
	if (!TEST_RETURN) {
		struct sockaddr_in *psin = 0;
		int err = 0;

		for (pai = aires; pai; pai = pai->ai_next) {
			err |= pai->ai_family != AF_INET;
			err |= pai->ai_addrlen != sizeof(struct sockaddr_in);
			err |= pai->ai_addr == 0;
			psin = (struct sockaddr_in *)pai->ai_addr;
			if (pai->ai_addr) {
				err |= psin->sin_family != AF_INET;
				err |= psin->sin_port != htons(servnum);
			}
			if (err)
				break;
		}
		if (err) {
			tst_resm(TFAIL, "getaddrinfo IPv4 host+service: "
				 "fam %d alen %d addr 0x%p addr/fam %d "
				 "addr/port %d H[%d]",
				 pai->ai_family, pai->ai_addrlen, psin,
				 psin ? psin->sin_family : 0,
				 psin ? psin->sin_port : 0,
				 psin ? htons(psin->sin_port) : 0);
			freeaddrinfo(aires);
			return;
		}
		tst_resm(TPASS, "getaddrinfo IPv4 host+service");
		freeaddrinfo(aires);
	} else {
		tst_resm(TFAIL, "getaddrinfo IPv4 host+"
			 "service returns %ld (\"%s\")", TEST_RETURN,
			 gai_strerror(TEST_RETURN));
		return;
	}

	/* test 4, IPv4 hostname+service, AI_PASSIVE */
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_flags = AI_PASSIVE;
	hints.ai_socktype = SOCK_STREAM;
	strcpy(service, "9462");
	servnum = htons(9462);
	TEST(getaddrinfo(hostname, service, &hints, &aires));
	if (!TEST_RETURN) {
		struct sockaddr_in *psin = 0;
		int err = 0;

		for (pai = aires; pai; pai = pai->ai_next) {
			err |= pai->ai_family != AF_INET;
			err |= pai->ai_addrlen != sizeof(struct sockaddr_in);
			err |= pai->ai_addr == 0;
			psin = (struct sockaddr_in *)pai->ai_addr;
			if (pai->ai_addr) {
				/* AI_PASSIVE is ignored if hostname is
				 * non-null; address must be set
				 */
				err |= psin->sin_addr.s_addr == 0;
				err |= psin->sin_family != AF_INET;
				err |= psin->sin_port != servnum;
			}
			if (err)
				break;
		}
		if (err) {
			tst_resm(TFAIL, "getaddrinfo IPv4 host+service, PASSIVE"
				 ": fam %d alen %d addr 0x%p addr/fam %d "
				 "addr/port %d H[%d]",
				 pai->ai_family, pai->ai_addrlen, psin,
				 psin ? psin->sin_family : 0,
				 psin ? psin->sin_port : 0,
				 psin ? htons(psin->sin_port) : 0);
			freeaddrinfo(aires);
			return;
		}
		tst_resm(TPASS, "getaddrinfo IPv4 host+service PASSIVE");
		freeaddrinfo(aires);
	} else {
		tst_resm(TFAIL, "getaddrinfo IPv4 host+"
			 "service, PASSIVE (\"%s\", \"%s\") returns %ld (\"%s\")",
			 hostname, service, TEST_RETURN,
			 gai_strerror(TEST_RETURN));
		return;
	}

	/* test 5, IPv4 host+service w/ AI_NUMERICHOST */
	memset(&hints, 0, sizeof(hints));
	strcpy(service, "echo");
	servnum = 7;
	hints.ai_family = AF_INET;
	hints.ai_flags = AI_NUMERICHOST;
	TEST(getaddrinfo(hostname, service, &hints, &aires));
	if (TEST_RETURN != EAI_NONAME) {
		tst_resm(TFAIL, "getaddrinfo IPv4 AI_NUMERICHOST w/ hostname: "
			 "returns %ld expected %d (EAI_NONAME)",
			 TEST_RETURN, EAI_NONAME);
		if (!TEST_RETURN)
			freeaddrinfo(aires);
		return;
	}
	tst_resm(TPASS, "getaddrinfo IPv4 AI_NUMERICHOST w/ hostname");
	if (!TEST_RETURN)
		freeaddrinfo(aires);

	/* test 6, IPv4 0+service, AI_PASSIVE */
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_flags = AI_PASSIVE;
	hints.ai_socktype = SOCK_STREAM;
	strcpy(service, "9462");
	servnum = htons(9462);
	TEST(getaddrinfo(0, service, &hints, &aires));
	if (!TEST_RETURN) {
		struct sockaddr_in *psin = 0;
		int err = 0;

		for (pai = aires; pai; pai = pai->ai_next) {
			err |= pai->ai_family != AF_INET;
			err |= pai->ai_addrlen != sizeof(struct sockaddr_in);
			err |= pai->ai_addr == 0;
			psin = (struct sockaddr_in *)pai->ai_addr;
			if (pai->ai_addr) {

				/* AI_PASSIVE means addr must be INADDR_ANY */
				err |= psin->sin_addr.s_addr != 0;
				err |= psin->sin_family != AF_INET;
				err |= psin->sin_port != servnum;
			}
			if (err)
				break;
		}
		if (err) {
			tst_resm(TFAIL, "getaddrinfo IPv4 0+service, PASSIVE:"
				 " fam %d alen %d addr 0x%p addr/fam %d "
				 "addr/port %d H[%d]",
				 pai->ai_family, pai->ai_addrlen, psin,
				 psin ? psin->sin_family : 0,
				 psin ? psin->sin_port : 0,
				 psin ? htons(psin->sin_port) : 0);
			freeaddrinfo(aires);
			return;
		}
		tst_resm(TPASS, "getaddrinfo IPv4 0+service, PASSIVE");
		freeaddrinfo(aires);
	} else {
		if (TEST_RETURN == EAI_BADFLAGS) {
			tst_resm(TPASS, "getaddrinfo IPv4 0+service,"
				" PASSIVE (\"\", \"%s\") returns %ld (\"%s\")",
				service, TEST_RETURN,
				gai_strerror(TEST_RETURN));
		} else {
			tst_resm(TFAIL, "getaddrinfo IPv4 0+service,"
				" PASSIVE (\"\", \"%s\") returns %ld (\"%s\")",
				service, TEST_RETURN,
				gai_strerror(TEST_RETURN));
			return;
		}
	}

	/* test 7, IPv4 0+service */
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	strcpy(service, "9462");
	servnum = htons(9462);
	TEST(getaddrinfo(0, service, &hints, &aires));
	if (!TEST_RETURN) {
		struct sockaddr_in *psin = 0;
		int err = 0;

		for (pai = aires; pai; pai = pai->ai_next) {
			err |= pai->ai_family != AF_INET;
			err |= pai->ai_addrlen != sizeof(struct sockaddr_in);
			err |= pai->ai_addr == 0;
			psin = (struct sockaddr_in *)pai->ai_addr;
			if (pai->ai_addr) {
				/* hostname not set; addr should be loopback */
				err |= psin->sin_addr.s_addr !=
				    htonl(INADDR_LOOPBACK);
				err |= psin->sin_family != AF_INET;
				err |= psin->sin_port != servnum;
			}
			if (err)
				break;
		}
		if (err) {
			tst_resm(TFAIL, "getaddrinfo IPv4 0+service: "
				 "fam %d alen %d addr 0x%p addr/fam %d "
				 "addr/port %d H[%d]",
				 pai->ai_family, pai->ai_addrlen, psin,
				 psin ? psin->sin_family : 0,
				 psin ? psin->sin_port : 0,
				 psin ? htons(psin->sin_port) : 0);
			freeaddrinfo(aires);
			return;
		}
		tst_resm(TPASS, "getaddrinfo IPv4 0+service");
		freeaddrinfo(aires);
	} else {
		if (TEST_RETURN == EAI_BADFLAGS) {
			tst_resm(TPASS, "getaddrinfo IPv4 "
				"0+service (\"\", \"%s\") returns %ld (\"%s\")",
				service, TEST_RETURN,
				gai_strerror(TEST_RETURN));
		} else {
			tst_resm(TFAIL, "getaddrinfo IPv4 "
				"0+service (\"\", \"%s\") returns %ld (\"%s\")",
				service, TEST_RETURN,
				gai_strerror(TEST_RETURN));
			return;
		}
	}

	/* test 8, IPv4 host+service, AI_NUMERICSERV */
#ifndef AI_NUMERICSERV
	tst_resm(TCONF, "getaddrinfo IPv4 host+service, AI_NUMERICSERV: flag "
		 "not implemented");
#else
	memset(&hints, 0, sizeof(hints));
	strcpy(service, "echo");
	servnum = 7;
	hints.ai_family = AF_INET;
	hints.ai_flags = AI_NUMERICSERV;
	TEST(getaddrinfo(hostname, service, &hints, &aires));
	if (TEST_RETURN != EAI_NONAME) {
		tst_resm(TFAIL,
			 "getaddrinfo IPv4 host+service, AI_NUMERICSERV: "
			 "returns %ld (\"%s\") expected %d (EAI_NONAME)",
			 TEST_RETURN, gai_strerror(TEST_RETURN), EAI_NONAME);
		if (!TEST_RETURN)
			freeaddrinfo(aires);
		return;
	}
	tst_resm(TPASS, "getaddrinfo IPv4 host+service, AI_NUMERICSERV");
	if (!TEST_RETURN)
		freeaddrinfo(aires);
#endif /* AI_NUMERICSERV */

	/* test 9, IPv4 SOCK_STREAM/IPPROTO_UDP hints */
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_UDP;
	strcpy(service, "9462");
	servnum = htons(9462);
	TEST(getaddrinfo(0, service, &hints, &aires));
	if (!TEST_RETURN) {
		tst_resm(TFAIL, "getaddrinfo IPv4 SOCK_STREAM/IPPROTO_UDP "
			 "hints");
		freeaddrinfo(aires);
		return;
	}
	tst_resm(TPASS, "getaddrinfo IPv4 SOCK_STREAM/IPPROTO_UDP hints");

	/* test 10, IPv4 socktype 0, 513 */
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = 0;
	strcpy(service, "513");
	servnum = htons(513);
	TEST(getaddrinfo(0, service, &hints, &aires));
	if (!TEST_RETURN) {
		struct sockaddr_in *psin = 0;
		int got_tcp, got_udp;
		int err = 0;

		got_tcp = got_udp = 0;
		for (pai = aires; pai; pai = pai->ai_next) {
			err |= pai->ai_family != AF_INET;
			err |= pai->ai_addrlen != sizeof(struct sockaddr_in);
			err |= pai->ai_addr == 0;
			got_tcp |= pai->ai_socktype == SOCK_STREAM;
			got_udp |= pai->ai_socktype == SOCK_DGRAM;
			psin = (struct sockaddr_in *)pai->ai_addr;
			if (pai->ai_addr) {
				/* hostname not set; addr should be loopback */
				err |= psin->sin_addr.s_addr !=
				    htonl(INADDR_LOOPBACK);
				err |= psin->sin_family != AF_INET;
				err |= psin->sin_port != servnum;
			}
			if (err)
				break;
		}
		if (err) {
			tst_resm(TFAIL, "getaddrinfo IPv4 socktype 0,513: "
				 "fam %d alen %d addr 0x%p addr/fam %d "
				 "addr/port %d H[%d]",
				 pai->ai_family, pai->ai_addrlen, psin,
				 psin ? psin->sin_family : 0,
				 psin ? psin->sin_port : 0,
				 psin ? htons(psin->sin_port) : 0);
			freeaddrinfo(aires);
			return;
		} else if (got_tcp && got_udp) {
			tst_resm(TPASS, "getaddrinfo IPv4 socktype 0,513");
			freeaddrinfo(aires);
		} else {
			tst_resm(TFAIL, "getaddrinfo IPv4 socktype 0,513 TCP %d"
				 " UDP %d", got_tcp, got_udp);
			freeaddrinfo(aires);
			return;
		}
	} else {
		if (TEST_RETURN == EAI_BADFLAGS) {
			tst_resm(TPASS, "getaddrinfo IPv4 socktype 0,513"
				" (\"\", \"%s\") returns %ld (\"%s\")", service,
				TEST_RETURN, gai_strerror(TEST_RETURN));
		} else {
			tst_resm(TFAIL, "getaddrinfo IPv4 socktype 0,513"
				" (\"\", \"%s\") returns %ld (\"%s\")", service,
				TEST_RETURN, gai_strerror(TEST_RETURN));
			return;
		}
	}

	/* test 11, IPv4 AI_V4MAPPED */
	/* AI_V4MAPPED should be ignored because family != AF_INET6 */
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_flags = AI_V4MAPPED;
	TEST(getaddrinfo(hostname, 0, &hints, &aires));
	if (!TEST_RETURN) {
		struct sockaddr_in *psin = 0;
		int err = 0;

		for (pai = aires; pai; pai = pai->ai_next) {
			err |= pai->ai_family != AF_INET;
			err |= pai->ai_addrlen != sizeof(struct sockaddr_in);
			err |= pai->ai_addr == 0;
			psin = (struct sockaddr_in *)pai->ai_addr;
			if (pai->ai_addr) {
				err |= psin->sin_family != AF_INET;
				err |= psin->sin_port != 0;
			}
			if (err)
				break;
		}
		if (err) {
			tst_resm(TFAIL, "getaddrinfo IPv4 AI_V4MAPPED: "
				 "fam %d alen %d addr 0x%p addr/fam %d "
				 "addr/port %d H[%d]",
				 pai->ai_family, pai->ai_addrlen, psin,
				 psin ? psin->sin_family : 0,
				 psin ? psin->sin_port : 0,
				 psin ? htons(psin->sin_port) : 0);
			freeaddrinfo(aires);
			return;
		}
		tst_resm(TPASS, "getaddrinfo IPv4 AI_V4MAPPED");
		freeaddrinfo(aires);
	} else {
		tst_resm(TFAIL, "getaddrinfo IPv4 "
			 "AI_V4MAPPED (\"%s\") returns %ld (\"%s\")", hostname,
			 TEST_RETURN, gai_strerror(TEST_RETURN));
		return;
	}
}

/* getaddrinfo tests (v6) */
static void gaiv6(void)
{
	struct addrinfo *aires, hints, *pai;
	char hostname[MAXHOSTNAMELEN + 1];
	char shortname[MAXHOSTNAMELEN + 1];
	char service[NI_MAXSERV + 1];
	int servnum;
	char *p;

	if (gethostname(hostname, sizeof(hostname)) < 0)
		tst_brkm(TBROK, NULL, "gethostname failed - %s",
			 strerror(errno));
	strncpy(shortname, hostname, MAXHOSTNAMELEN);
	shortname[MAXHOSTNAMELEN] = '\0';
	p = strchr(shortname, '.');
	if (p)
		*p = '\0';

	/* test 12, IPv6 basic lookup */
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET6;
	TEST(getaddrinfo(hostname, 0, &hints, &aires));
	if (!TEST_RETURN) {
		struct sockaddr_in6 *psin6 = 0;
		int err = 0;

		for (pai = aires; pai; pai = pai->ai_next) {
			err |= pai->ai_family != AF_INET6;
			err |= pai->ai_addrlen != sizeof(struct sockaddr_in6);
			err |= pai->ai_addr == 0;
			psin6 = (struct sockaddr_in6 *)pai->ai_addr;
			if (pai->ai_addr) {
				err |= psin6->sin6_family != AF_INET6;
				err |= psin6->sin6_port != 0;
			}
			if (err)
				break;
		}
		if (err) {
			tst_resm(TFAIL, "getaddrinfo IPv6 basic lookup: "
				 "fam %d alen %d addr 0x%p addr/fam %d "
				 "addr/port %d H[%d]",
				 pai->ai_family, pai->ai_addrlen, psin6,
				 psin6 ? psin6->sin6_family : 0,
				 psin6 ? psin6->sin6_port : 0,
				 psin6 ? htons(psin6->sin6_port) : 0);
			freeaddrinfo(aires);
			return;
		}
		tst_resm(TPASS, "getaddrinfo IPv6 basic lookup");
		freeaddrinfo(aires);
	} else {
		tst_resm(TFAIL, "getaddrinfo IPv6 basic "
			 "lookup (\"%s\") returns %ld (\"%s\")", hostname,
			 TEST_RETURN, gai_strerror(TEST_RETURN));
		return;
	}

	/* test 13, IPv6 canonical name */
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET6;
	hints.ai_flags = AI_CANONNAME;
	TEST(getaddrinfo(shortname, 0, &hints, &aires));
	if (!TEST_RETURN) {
		for (pai = aires; pai; pai = pai->ai_next)
			if (pai->ai_canonname)
				break;
		if (!pai) {
			tst_resm(TFAIL, "getaddrinfo IPv6 canonical name: no "
				 "entries with canonical name set");
			freeaddrinfo(aires);
			return;
		} else if (strcasecmp(hostname, pai->ai_canonname)) {
			tst_resm(TFAIL, "getaddrinfo IPv6 canonical name "
				 "(\"%s\") doesn't match hostname (\"%s\")",
				 pai->ai_canonname, hostname);
			freeaddrinfo(aires);
			return;
		}
		tst_resm(TPASS, "getaddrinfo IPv6 canonical name");
		freeaddrinfo(aires);
	} else {
		tst_resm(TFAIL, "getaddrinfo IPv6 "
			 "canonical name (\"%s\") returns %ld (\"%s\")",
			 shortname, TEST_RETURN, gai_strerror(TEST_RETURN));
		return;
	}

	/* test 14, IPv6 host+service name */
	memset(&hints, 0, sizeof(hints));
	/*
	 * These are hard-coded for echo/7 to avoid using getservbyname(),
	 * since it isn't thread-safe and these tests may be re-used
	 * multithreaded. Sigh.
	 */
	strcpy(service, "echo");
	servnum = 7;
	hints.ai_family = AF_INET6;
	TEST(getaddrinfo(hostname, service, &hints, &aires));
	if (!TEST_RETURN) {
		struct sockaddr_in6 *psin6 = 0;
		int err = 0;

		for (pai = aires; pai; pai = pai->ai_next) {
			err |= pai->ai_family != AF_INET6;
			err |= pai->ai_addrlen != sizeof(struct sockaddr_in6);
			err |= pai->ai_addr == 0;
			psin6 = (struct sockaddr_in6 *)pai->ai_addr;
			if (pai->ai_addr) {
				err |= psin6->sin6_family != AF_INET6;
				err |= psin6->sin6_port != htons(servnum);
			}
			if (err)
				break;
		}
		if (err) {
			tst_resm(TFAIL, "getaddrinfo IPv6 host+service: "
				 "fam %d alen %d addr 0x%p addr/fam %d "
				 "addr/port %d H[%d]",
				 pai->ai_family, pai->ai_addrlen, psin6,
				 psin6 ? psin6->sin6_family : 0,
				 psin6 ? psin6->sin6_port : 0,
				 psin6 ? htons(psin6->sin6_port) : 0);
			freeaddrinfo(aires);
			return;
		}
		tst_resm(TPASS, "getaddrinfo IPv6 host+service");
		freeaddrinfo(aires);
	} else {
		tst_resm(TFAIL, "getaddrinfo IPv6 host+"
			 "service returns %ld (\"%s\")", TEST_RETURN,
			 gai_strerror(TEST_RETURN));
		return;
	}

	/* test 15, IPv6 hostname+service, AI_PASSIVE */
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET6;
	hints.ai_flags = AI_PASSIVE;
	hints.ai_socktype = SOCK_STREAM;
	strcpy(service, "9462");
	servnum = htons(9462);
	TEST(getaddrinfo(hostname, service, &hints, &aires));
	if (!TEST_RETURN) {
		struct sockaddr_in6 *psin6 = 0;
		int err = 0;

		for (pai = aires; pai; pai = pai->ai_next) {
			err |= pai->ai_family != AF_INET6;
			err |= pai->ai_addrlen != sizeof(struct sockaddr_in6);
			err |= pai->ai_addr == 0;
			psin6 = (struct sockaddr_in6 *)pai->ai_addr;
			if (pai->ai_addr) {
				/* AI_PASSIVE is ignored if hostname is
				 * non-null; address must be set
				 */
				err |= memcmp(&psin6->sin6_addr, &in6addr_any,
					      sizeof(struct in6_addr)) == 0;
				err |= psin6->sin6_family != AF_INET6;
				err |= psin6->sin6_port != servnum;
			}
			if (err)
				break;
		}
		if (err) {
			tst_resm(TFAIL, "getaddrinfo IPv6 host+service, PASSIVE"
				 ": fam %d alen %d addr 0x%p addr/fam %d "
				 "addr/port %d H[%d]",
				 pai->ai_family, pai->ai_addrlen, psin6,
				 psin6 ? psin6->sin6_family : 0,
				 psin6 ? psin6->sin6_port : 0,
				 psin6 ? htons(psin6->sin6_port) : 0);
			freeaddrinfo(aires);
			return;
		}
		tst_resm(TPASS, "getaddrinfo IPv6 host+service PASSIVE");
		freeaddrinfo(aires);
	} else {
		tst_resm(TFAIL, "getaddrinfo IPv6 host+"
			 "service, PASSIVE (\"%s\", \"%s\") returns %ld (\"%s\")",
			 hostname, service, TEST_RETURN,
			 gai_strerror(TEST_RETURN));
		return;
	}

	/* test 16, IPv6 host+service w/ AI_NUMERICHOST */
	memset(&hints, 0, sizeof(hints));
	strcpy(service, "echo");
	servnum = 7;
	hints.ai_family = AF_INET6;
	hints.ai_flags = AI_NUMERICHOST;
	TEST(getaddrinfo(hostname, service, &hints, &aires));
	if (TEST_RETURN != EAI_NONAME) {
		tst_resm(TFAIL, "getaddrinfo IPv6 AI_NUMERICHOST w/ hostname: "
			 "returns %ld expected %d (EAI_NONAME)",
			 TEST_RETURN, EAI_NONAME);
		if (!TEST_RETURN)
			freeaddrinfo(aires);
		return;
	}
	tst_resm(TPASS, "getaddrinfo IPv6 AI_NUMERICHOST w/ hostname");
	if (!TEST_RETURN)
		freeaddrinfo(aires);

	/* test 17, IPv6 0+service, AI_PASSIVE */
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET6;
	hints.ai_flags = AI_PASSIVE;
	hints.ai_socktype = SOCK_STREAM;
	strcpy(service, "9462");
	servnum = htons(9462);
	TEST(getaddrinfo(0, service, &hints, &aires));
	if (!TEST_RETURN) {
		struct sockaddr_in6 *psin6 = 0;
		int err = 0;

		for (pai = aires; pai; pai = pai->ai_next) {
			err |= pai->ai_family != AF_INET6;
			err |= pai->ai_addrlen != sizeof(struct sockaddr_in6);
			err |= pai->ai_addr == 0;
			psin6 = (struct sockaddr_in6 *)pai->ai_addr;
			if (pai->ai_addr) {

				/* AI_PASSIVE means addr must be INADDR_ANY */
				err |= memcmp(&psin6->sin6_addr, &in6addr_any,
					      sizeof(struct in6_addr)) != 0;
				err |= psin6->sin6_family != AF_INET6;
				err |= psin6->sin6_port != servnum;
			}
			if (err)
				break;
		}
		if (err) {
			tst_resm(TFAIL, "getaddrinfo IPv6 0+service, PASSIVE:"
				 " fam %d alen %d addr 0x%p addr/fam %d "
				 "addr/port %d H[%d]",
				 pai->ai_family, pai->ai_addrlen, psin6,
				 psin6 ? psin6->sin6_family : 0,
				 psin6 ? psin6->sin6_port : 0,
				 psin6 ? htons(psin6->sin6_port) : 0);
			freeaddrinfo(aires);
			return;
		}
		tst_resm(TPASS, "getaddrinfo IPv6 0+service, PASSIVE");
		freeaddrinfo(aires);
	} else {
		if (TEST_RETURN == EAI_BADFLAGS) {
			tst_resm(TPASS, "getaddrinfo IPv6 0+service, PASSIVE"
				" (\"\", \"%s\") returns %ld (\"%s\")", service,
				TEST_RETURN, gai_strerror(TEST_RETURN));
		} else {
			tst_resm(TFAIL, "getaddrinfo IPv6 0+service, PASSIVE"
				" (\"\", \"%s\") returns %ld (\"%s\")", service,
				TEST_RETURN, gai_strerror(TEST_RETURN));
			return;
		}
	}

	/* test 18, IPv6 0+service */
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET6;
	hints.ai_socktype = SOCK_STREAM;
	strcpy(service, "9462");
	servnum = htons(9462);
	TEST(getaddrinfo(0, service, &hints, &aires));
	if (!TEST_RETURN) {
		struct sockaddr_in6 *psin6 = 0;
		int err = 0;

		for (pai = aires; pai; pai = pai->ai_next) {
			err |= pai->ai_family != AF_INET6;
			err |= pai->ai_addrlen != sizeof(struct sockaddr_in6);
			err |= pai->ai_addr == 0;
			psin6 = (struct sockaddr_in6 *)pai->ai_addr;
			if (pai->ai_addr) {
				/* hostname not set; addr should be loopback */
				err |= memcmp(&psin6->sin6_addr,
					      &in6addr_loopback,
					      sizeof(struct in6_addr)) != 0;
				err |= psin6->sin6_family != AF_INET6;
				err |= psin6->sin6_port != servnum;
			}
			if (err)
				break;
		}
		if (err) {
			tst_resm(TFAIL, "getaddrinfo IPv6 0+service: "
				 "fam %d alen %d addr 0x%p addr/fam %d "
				 "addr/port %d H[%d]",
				 pai->ai_family, pai->ai_addrlen, psin6,
				 psin6 ? psin6->sin6_family : 0,
				 psin6 ? psin6->sin6_port : 0,
				 psin6 ? htons(psin6->sin6_port) : 0);
			freeaddrinfo(aires);
			return;
		}
		tst_resm(TPASS, "getaddrinfo IPv6 0+service");
		freeaddrinfo(aires);
	} else {
		if (TEST_RETURN == EAI_BADFLAGS) {
			tst_resm(TPASS, "getaddrinfo IPv6 0+service"
				" (\"\", \"%s\") returns %ld (\"%s\")", service,
				TEST_RETURN, gai_strerror(TEST_RETURN));
		} else {
			tst_resm(TFAIL, "getaddrinfo IPv6 0+service"
				" (\"\", \"%s\") returns %ld (\"%s\")", service,
				TEST_RETURN, gai_strerror(TEST_RETURN));
			return;
		}
	}

	/* test 19, IPv6 host+service, AI_NUMERICSERV */
#ifndef AI_NUMERICSERV
	tst_resm(TCONF, "getaddrinfo IPv6 host+service, AI_NUMERICSERV: flag "
		 "not implemented");
#else
	memset(&hints, 0, sizeof(hints));
	strcpy(service, "echo");
	servnum = 7;
	hints.ai_family = AF_INET6;
	hints.ai_flags = AI_NUMERICSERV;
	TEST(getaddrinfo(hostname, service, &hints, &aires));
	if (TEST_RETURN != EAI_NONAME) {
		tst_resm(TFAIL,
			 "getaddrinfo IPv6 host+service, AI_NUMERICSERV: "
			 "returns %ld (\"%s\") expected %d (EAI_NONAME)",
			 TEST_RETURN, gai_strerror(TEST_RETURN), EAI_NONAME);
		if (!TEST_RETURN)
			freeaddrinfo(aires);
		return;
	}
	tst_resm(TPASS, "getaddrinfo IPv6 host+service, AI_NUMERICSERV");
	if (!TEST_RETURN)
		freeaddrinfo(aires);
#endif /* AI_NUMERICSERV */

	/* test 20, IPv6 SOCK_STREAM/IPPROTO_UDP hints */
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET6;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_UDP;
	strcpy(service, "9462");
	servnum = htons(9462);
	TEST(getaddrinfo(0, service, &hints, &aires));
	if (!TEST_RETURN) {
		tst_resm(TFAIL, "getaddrinfo IPv6 SOCK_STREAM/IPPROTO_UDP "
			 "hints");
		freeaddrinfo(aires);
		return;
	}
	tst_resm(TPASS, "getaddrinfo IPv6 SOCK_STREAM/IPPROTO_UDP hints");

	/* test 21, IPv6 socktype 0, 513 */
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET6;
	hints.ai_socktype = 0;
	strcpy(service, "513");
	servnum = htons(513);
	TEST(getaddrinfo(0, service, &hints, &aires));
	if (!TEST_RETURN) {
		struct sockaddr_in6 *psin6 = 0;
		int got_tcp, got_udp;
		int err = 0;

		got_tcp = got_udp = 0;
		for (pai = aires; pai; pai = pai->ai_next) {
			err |= pai->ai_family != AF_INET6;
			err |= pai->ai_addrlen != sizeof(struct sockaddr_in6);
			err |= pai->ai_addr == 0;
			got_tcp |= pai->ai_socktype == SOCK_STREAM;
			got_udp |= pai->ai_socktype == SOCK_DGRAM;
			psin6 = (struct sockaddr_in6 *)pai->ai_addr;
			if (pai->ai_addr) {
				/* hostname not set; addr should be loopback */
				err |= memcmp(&psin6->sin6_addr,
					      &in6addr_loopback,
					      sizeof(struct in6_addr)) != 0;
				err |= psin6->sin6_family != AF_INET6;
				err |= psin6->sin6_port != servnum;
			}
			if (err)
				break;
		}
		if (err) {
			tst_resm(TFAIL, "getaddrinfo IPv6 socktype 0,513: "
				 "fam %d alen %d addr 0x%p addr/fam %d "
				 "addr/port %d H[%d]",
				 pai->ai_family, pai->ai_addrlen, psin6,
				 psin6 ? psin6->sin6_family : 0,
				 psin6 ? psin6->sin6_port : 0,
				 psin6 ? htons(psin6->sin6_port) : 0);
			freeaddrinfo(aires);
			return;
		} else if (got_tcp && got_udp) {
			tst_resm(TPASS, "getaddrinfo IPv6 socktype 0,513");
			freeaddrinfo(aires);
		} else {
			tst_resm(TFAIL, "getaddrinfo IPv6 socktype 0,513 TCP %d"
				 " UDP %d", got_tcp, got_udp);
			freeaddrinfo(aires);
			return;
		}
	} else {
		if (TEST_RETURN == EAI_BADFLAGS) {
			tst_resm(TPASS, "getaddrinfo IPv6 socktype 0,513"
				" (\"\", \"%s\") returns %ld (\"%s\")", service,
				TEST_RETURN, gai_strerror(TEST_RETURN));
		} else {
			tst_resm(TFAIL, "getaddrinfo IPv6 socktype 0,513"
				" (\"\", \"%s\") returns %ld (\"%s\")", service,
				TEST_RETURN, gai_strerror(TEST_RETURN));
			return;
		}
	}

	/* test 22, IPv6 AI_V4MAPPED */
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET6;
	hints.ai_flags = AI_V4MAPPED;
	TEST(getaddrinfo(hostname, 0, &hints, &aires));
	if (!TEST_RETURN) {
		struct sockaddr_in6 *psin6 = 0;
		int err = 0;

		for (pai = aires; pai; pai = pai->ai_next) {
			err |= pai->ai_family != AF_INET6;
			err |= pai->ai_addrlen != sizeof(struct sockaddr_in6);
			err |= pai->ai_addr == 0;
			psin6 = (struct sockaddr_in6 *)pai->ai_addr;
			if (pai->ai_addr) {
				err |= psin6->sin6_family != AF_INET6;
				err |= psin6->sin6_port != 0;
			}
			if (err)
				break;
		}
		if (err) {
			tst_resm(TFAIL, "getaddrinfo IPv6 AI_V4MAPPED: "
				 "fam %d alen %d addr 0x%p addr/fam %d "
				 "addr/port %d H[%d]",
				 pai->ai_family, pai->ai_addrlen, psin6,
				 psin6 ? psin6->sin6_family : 0,
				 psin6 ? psin6->sin6_port : 0,
				 psin6 ? htons(psin6->sin6_port) : 0);
			freeaddrinfo(aires);
			return;
		}
		tst_resm(TPASS, "getaddrinfo IPv6 AI_V4MAPPED");
		freeaddrinfo(aires);
	} else {
		tst_resm(TFAIL, "getaddrinfo IPv6 "
			 "AI_V4MAPPED (\"%s\") returns %ld (\"%s\")", hostname,
			 TEST_RETURN, gai_strerror(TEST_RETURN));
		return;
	}
}
