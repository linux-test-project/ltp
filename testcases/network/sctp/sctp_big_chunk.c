/*
 * Copyright (c) 2018 Oracle and/or its affiliates. All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it would be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 * Regression test-case for the crash caused by over-sized SCTP chunk,
 * fixed by upstream commit 07f2c7ab6f8d ("sctp: verify size of a new
 * chunk in _sctp_make_chunk()")
 */

#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/syscall.h>
#include <fcntl.h>

#include "tst_test.h"
#include "tst_safe_stdio.h"
#include "lapi/netinet_in.h"
#include "lapi/socket.h"
#include "lapi/sctp.h"

static int port;
static int sfd, cfd;
static struct sockaddr_in6 rmt, loc;

static char *addr_param;
static int addr_num = 3273;

static void setup_server(void)
{
	loc.sin6_family = AF_INET6;
	loc.sin6_addr = in6addr_loopback;

	sfd = SAFE_SOCKET(AF_INET6, SOCK_STREAM, IPPROTO_SCTP);
	SAFE_BIND(sfd, (struct sockaddr *)&loc, sizeof(loc));

	port = TST_GETSOCKPORT(sfd);
	tst_res(TINFO, "sctp server listen on %d", port);

	SAFE_LISTEN(sfd, 1);
}

static void setup_client(void)
{
	struct sockaddr_in6 addr_buf[addr_num];
	int i;

	cfd = SAFE_SOCKET(AF_INET6, SOCK_STREAM, IPPROTO_SCTP);
	rmt.sin6_family = AF_INET6;
	rmt.sin6_addr = in6addr_loopback;
	rmt.sin6_port = htons(port);

	tst_res(TINFO, "bind %d additional IP addresses", addr_num);

	memset(addr_buf, 0, sizeof(addr_buf));
	for (i = 0; i < addr_num; ++i) {
		addr_buf[i].sin6_family = AF_INET6;
		addr_buf[i].sin6_addr = in6addr_loopback;
	}

	SAFE_SETSOCKOPT(cfd, SOL_SCTP, SCTP_SOCKOPT_BINDX_ADD, addr_buf,
			sizeof(addr_buf));
}

static void setup(void)
{
	if (tst_parse_int(addr_param, &addr_num, 1, INT_MAX))
		tst_brk(TBROK, "wrong address number '%s'", addr_param);

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

	fcntl(cfd, F_SETFL, O_NONBLOCK);
	connect(cfd, (struct sockaddr *)&rmt, sizeof(rmt));

	SAFE_KILL(pid, SIGKILL);
	SAFE_WAITPID(pid, NULL, 0);

	tst_res(TPASS, "test doesn't cause crash");
}

static struct tst_option options[] = {
	{"a:", &addr_param, "-a       number of additional IP address params"},
	{NULL, NULL, NULL}
};

static struct tst_test test = {
	.setup = setup,
	.forks_child = 1,
	.test_all = run,
	.options = options
};
