// SPDX-License-Identifier: GPL-2.0-or-later
/* SCTP kernel Implementation
 * Copyright (c) 2003 Hewlett-Packard Development Company, L.P
 * (C) Copyright IBM Corp. 2004
 * Copyright (c) 2019 Martin Doucha <mdoucha@suse.cz>
 *
 * When init timeout is set to zero, a connect () crashed the system. This case
 * tests the fix for the same.
 */

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>         /* for sockaddr_in */
#include <arpa/inet.h>
#include <netinet/sctp.h>
#include "tst_test.h"
#include "tst_net.h"

#ifdef PROT_SOCK
#define SCTP_TESTPORT_1 PROT_SOCK
#else
#define SCTP_TESTPORT_1 1024
#endif

#define SCTP_IP_LOOPBACK  htonl(0x7f000001)

static const struct test_case {
	__u16 streams;
	int accept_err;
} testcase_list[] = {
	{10, 0},
	{65535, ENOMEM}
};

static void test_sctp(unsigned int n)
{
	int sk1, sk2, sk3, msglen;
	socklen_t len;
	struct sockaddr_in lstn_addr, acpt_addr;
	char *buffer_rcv;
	struct sctp_initmsg sinmsg;
	const char *message = "Hello World!\n";
	const struct test_case *tc = testcase_list + n;

	tst_res(TINFO, "Running test with %u streams", tc->streams);

	sk1 = SAFE_SOCKET(PF_INET, SOCK_STREAM, IPPROTO_SCTP);
	sk3 = SAFE_SOCKET(PF_INET, SOCK_STREAM, IPPROTO_SCTP);

	lstn_addr.sin_family = AF_INET;
	lstn_addr.sin_addr.s_addr = SCTP_IP_LOOPBACK;
	lstn_addr.sin_port = htons(SCTP_TESTPORT_1);

	SAFE_BIND(sk3, (struct sockaddr *) &lstn_addr, sizeof(lstn_addr));

	len = sizeof(struct sctp_initmsg);
	sinmsg.sinit_num_ostreams = tc->streams;
	sinmsg.sinit_max_instreams = 10;
	sinmsg.sinit_max_attempts = 1;
	sinmsg.sinit_max_init_timeo = 0;
	SAFE_SETSOCKOPT(sk1, SOL_SCTP, SCTP_INITMSG, &sinmsg, len);
	sinmsg.sinit_num_ostreams = 10;
	sinmsg.sinit_max_instreams = tc->streams;
	SAFE_SETSOCKOPT(sk3, SOL_SCTP, SCTP_INITMSG, &sinmsg, len);

	SAFE_LISTEN(sk3, 1);

	len = sizeof(struct sockaddr_in);
	TEST(connect(sk1, (struct sockaddr *) &lstn_addr, len));

	if (TST_RET == -1 && tc->accept_err && TST_ERR == tc->accept_err) {
		tst_res(TPASS, "connect() failed in an acceptable way");
		SAFE_CLOSE(sk1);
		SAFE_CLOSE(sk3);
		return;
	} else if (TST_RET < 0) {
		tst_brk(TBROK | TTERRNO, "connect() failed");
	}

	sk2 = SAFE_ACCEPT(sk3, (struct sockaddr *) &acpt_addr, &len);

	msglen = strlen(message) + 1;
	TEST(sctp_sendmsg(sk1, message, msglen, (struct sockaddr *)&lstn_addr,
		len, 0, 0, tc->streams - 1, 0, 0));

	if (TST_RET != msglen) {
		tst_brk(TBROK | TTERRNO, "sctp_sendmsg() failed");
	}

	buffer_rcv = malloc(msglen);
	TEST(recv(sk2, buffer_rcv, msglen, MSG_NOSIGNAL));

	if (TST_RET != msglen || strncmp(buffer_rcv, message, msglen)) {
		tst_res(TFAIL | TTERRNO, "recv() failed");
	} else {
		tst_res(TPASS, "connect() with init timeout set to 0");
	}

	free(buffer_rcv);
	SAFE_CLOSE(sk1);
	SAFE_CLOSE(sk2);
	SAFE_CLOSE(sk3);
}

static struct tst_test test = {
	.test = test_sctp,
	.tcnt = ARRAY_SIZE(testcase_list),
};
