/*
 * Copyright (c) 2016 Cyril Hrubis <chrubis@suse.cz>
 * Copyright (c) 2016 Michal Kubecek <mkubecek@suse.cz>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

/*
 * This is a regression test for kernel commit:
 *
 * 197c949e7798  udp: properly support MSG_PEEK with truncated buffers
 *
 * NOTE: The testcase will hang on upatched stable kernel.
 */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "test.h"
#include "safe_macros.h"

const char *TCID = "recvmsg02";
int TST_TOTAL = 1;

static const char msg[] = "Michael Gilfix was here\341\210\264\r\n";
static const unsigned msglen = ARRAY_SIZE(msg) - 1;
static unsigned char buff[25];
static const int bufflen = ARRAY_SIZE(buff);

void run_test(void)
{
	struct sockaddr_in6 addr_init = {
		.sin6_family	= AF_INET6,
		.sin6_port	= htons(0),
		.sin6_addr	= IN6ADDR_LOOPBACK_INIT,
	};
	struct sockaddr_in6 addr_r, addr_w, addr_f;
	socklen_t addrlen_r, addrlen_w;
	struct iovec iov = {
		.iov_base	= buff,
		.iov_len	= sizeof(buff),
	};
	struct msghdr msghdr = {
		.msg_name	= &addr_f,
		.msg_namelen	= sizeof(addr_f),
		.msg_iov	= &iov,
		.msg_iovlen	= 1,
		.msg_control	= NULL,
		.msg_controllen	= 0,
		.msg_flags	= 0,
	};
	int sdr, sdw;
	int R;

	sdr = SAFE_SOCKET(NULL, PF_INET6, SOCK_DGRAM | SOCK_CLOEXEC, IPPROTO_IP);
	SAFE_BIND(NULL, sdr, (struct sockaddr*)&addr_init, sizeof(addr_init));
	addrlen_r = sizeof(addr_r);
	SAFE_GETSOCKNAME(NULL, sdr, (struct sockaddr*)&addr_r, &addrlen_r);
	sdw = SAFE_SOCKET(NULL, PF_INET6, SOCK_DGRAM|SOCK_CLOEXEC, IPPROTO_IP);
	SAFE_BIND(NULL, sdw, (struct sockaddr*)&addr_init, sizeof(addr_init));
	addrlen_w = sizeof(addr_w);
	SAFE_GETSOCKNAME(NULL, sdw, (struct sockaddr*)&addr_w, &addrlen_w);

	R = sendto(sdw, msg, msglen, 0, (struct sockaddr*)&addr_r, addrlen_r);
	if (R < 0)
		tst_brkm(TBROK | TERRNO, NULL, "sendto()");

	R = recvmsg(sdr, &msghdr, MSG_PEEK);
	if (R < 0) {
		tst_resm(TFAIL | TERRNO, "recvmsg(..., MSG_PEEK)");
		return;
	}

	tst_resm(TINFO, "received %d bytes", R);

	if ((R == bufflen) && !memcmp(msg, buff, R))
		tst_resm(TPASS, "recvmsg(..., MSG_PEEK) works fine");
	else
		tst_resm(TPASS, "recvmsg(..., MSG_PEEK) failed");

	close(sdw);
	close(sdr);
}

int main(int argc, char *argv[])
{
	int lc;

	tst_parse_opts(argc, argv, NULL, NULL);

	for (lc = 0; TEST_LOOPING(lc); ++lc)
		run_test();

	tst_exit();
}
