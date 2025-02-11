// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2020 SUSE LLC
 * Author: Nicolai Stange <nstange@suse.de>
 * LTP port: Martin Doucha <mdoucha@suse.cz>
 */

/*\
 * Test for CVE-2020-25705 fixed in kernel v5.10:
 * b38e7819cae9 ("icmp: randomize the global rate limiter").
 *
 * Test of ICMP rate limiting behavior that may be abused for DNS cache
 * poisoning attack. Send a few batches of 100 packets to a closed UDP port
 * and count the ICMP errors. If the number of errors is always the same
 * for each batch (not randomized), the system is vulnerable. Send packets
 * from multiple IP addresses to bypass per-address ICMP throttling.
 */

#include <time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <linux/errqueue.h>

#include <limits.h>

#include "lapi/if_addr.h"
#include "tst_test.h"
#include "tst_netdevice.h"
#include "lapi/sched.h"

#define DSTNET 0xfa444e00 /* 250.68.78.0 */
#define SRCNET 0xfa444e40 /* 250.68.78.64 */
#define DSTADDR 0xfa444e02 /* 250.68.78.2 */
#define SRCADDR_BASE 0xfa444e41 /* 250.68.78.65 */
#define SRCADDR_COUNT 50
#define NETMASK 26
#define BATCH_COUNT 8
#define BUFSIZE 1024

static int parentns = -1, childns = -1;
static int fds[SRCADDR_COUNT];

static void setup(void)
{
	struct sockaddr_in ipaddr = { .sin_family = AF_INET };
	uint32_t addr;
	int i;

	for (i = 0; i < SRCADDR_COUNT; i++)
		fds[i] = -1;

	tst_setup_netns();

	/*
	 * Create network namespace to hide the destination interface from
	 * the test process.
	 */
	parentns = SAFE_OPEN("/proc/self/ns/net", O_RDONLY);
	SAFE_UNSHARE(CLONE_NEWNET);

	/* Do NOT close this FD, or both interfaces will be destroyed */
	childns = SAFE_OPEN("/proc/self/ns/net", O_RDONLY);

	/* Configure child namespace */
	CREATE_VETH_PAIR("ltp_veth1", "ltp_veth2");
	NETDEV_ADD_ADDRESS_INET("ltp_veth2", htonl(DSTADDR), NETMASK,
		IFA_F_NOPREFIXROUTE);
	NETDEV_SET_STATE("ltp_veth2", 1);
	NETDEV_ADD_ROUTE_INET("ltp_veth2", 0, 0, htonl(SRCNET), NETMASK, 0);

	/* Configure parent namespace */
	NETDEV_CHANGE_NS_FD("ltp_veth1", parentns);
	SAFE_SETNS(parentns, CLONE_NEWNET);
	addr = SRCADDR_BASE;

	for (i = 0; i < SRCADDR_COUNT; i++, addr++) {
		NETDEV_ADD_ADDRESS_INET("ltp_veth1", htonl(addr), NETMASK,
			IFA_F_NOPREFIXROUTE);
	}

	NETDEV_SET_STATE("ltp_veth1", 1);
	NETDEV_ADD_ROUTE_INET("ltp_veth1", 0, 0, htonl(DSTNET), NETMASK, 0);
	SAFE_FILE_PRINTF("/proc/sys/net/ipv4/conf/ltp_veth1/forwarding", "1");

	/* Open test sockets */
	for (i = 0; i < SRCADDR_COUNT; i++) {
		ipaddr.sin_addr.s_addr = htonl(SRCADDR_BASE + i);
		fds[i] = SAFE_SOCKET(AF_INET, SOCK_DGRAM, 0);
		SAFE_SETSOCKOPT_INT(fds[i], IPPROTO_IP, IP_RECVERR, 1);
		SAFE_BIND(fds[i], (struct sockaddr *)&ipaddr, sizeof(ipaddr));
	}
}

static int count_icmp_errors(int fd)
{
	int error_count = 0;
	ssize_t len;
	char msgbuf[BUFSIZE], errbuf[BUFSIZE];
	struct sockaddr_in addr;
	struct cmsghdr *cmsg;
	struct sock_extended_err exterr;
	struct iovec iov = {
		.iov_base = msgbuf,
		.iov_len = BUFSIZE
	};

	while (1) {
		struct msghdr msg = {
			.msg_name = (struct sockaddr *)&addr,
			.msg_namelen = sizeof(addr),
			.msg_iov = &iov,
			.msg_iovlen = 1,
			.msg_flags = 0,
			.msg_control = errbuf,
			.msg_controllen = BUFSIZE
		};

		memset(errbuf, 0, BUFSIZE);
		errno = 0;
		len = recvmsg(fd, &msg, MSG_ERRQUEUE);

		if (len == -1) {
			if (errno == EWOULDBLOCK || errno == EAGAIN)
				break;

			tst_brk(TBROK | TERRNO, "recvmsg() failed");
		}

		if (len < 0) {
			tst_brk(TBROK | TERRNO,
				"Invalid recvmsg() return value %zd", len);
		}

		for (cmsg = CMSG_FIRSTHDR(&msg); cmsg;
			cmsg = CMSG_NXTHDR(&msg, cmsg)) {
			if (cmsg->cmsg_level != SOL_IP)
				continue;

			if (cmsg->cmsg_type != IP_RECVERR)
				continue;

			memcpy(&exterr, CMSG_DATA(cmsg), sizeof(exterr));

			if (exterr.ee_origin != SO_EE_ORIGIN_ICMP)
				tst_brk(TBROK, "Unexpected non-ICMP error");

			if (exterr.ee_errno != ECONNREFUSED) {
				TST_ERR = exterr.ee_errno;
				tst_brk(TBROK | TTERRNO,
					"Unexpected ICMP error");
			}

			error_count++;
		}
	}

	return error_count;
}

static int packet_batch(const struct sockaddr *addr, socklen_t addrsize)
{
	int i, j, error_count = 0;
	char data = 0;

	for (i = 0; i < SRCADDR_COUNT; i++) {
		for (j = 0; j < 2; j++) {
			error_count += count_icmp_errors(fds[i]);
			TEST(sendto(fds[i], &data, sizeof(data), 0, addr,
				addrsize));

			if (TST_RET == -1) {
				if (TST_ERR == ECONNREFUSED) {
					j--; /* flush ICMP errors and retry */
					continue;
				}

				tst_brk(TBROK | TTERRNO, "sento() failed");
			}

			if (TST_RET < 0) {
				tst_brk(TBROK | TTERRNO,
					"Invalid sento() return value %ld",
					TST_RET);
			}
		}
	}

	/*
	 * Wait and collect pending ICMP errors. Waiting less than 2 seconds
	 * will make the test unreliable. Looping over each socket multiple
	 * times (with or without poll()) will cause kernel to silently
	 * discard ICMP errors, allowing the test to pass on vulnerable
	 * systems.
	 */
	sleep(2);

	for (i = 0; i < SRCADDR_COUNT; i++)
		error_count += count_icmp_errors(fds[i]);

	return error_count;
}

static void run(void)
{
	int i, errors_baseline, errors;
	struct sockaddr_in addr = {
		.sin_family = AF_INET,
		.sin_port = TST_GET_UNUSED_PORT(AF_INET, SOCK_DGRAM),
		.sin_addr = { htonl(DSTADDR) }
	};

	errors_baseline = packet_batch((struct sockaddr *)&addr, sizeof(addr));
	errors = errors_baseline;
	tst_res(TINFO, "Batch 0: Got %d ICMP errors", errors);

	for (i = 1; i < BATCH_COUNT && errors == errors_baseline; i++) {
		errors = packet_batch((struct sockaddr *)&addr, sizeof(addr));
		tst_res(TINFO, "Batch %d: Got %d ICMP errors", i, errors);
	}

	if (errors == errors_baseline) {
		tst_res(TFAIL,
			"ICMP rate limit not randomized, system is vulnerable");
		return;
	}

	tst_res(TPASS, "ICMP rate limit is randomized");
}

static void cleanup(void)
{
	int i;

	for (i = 0; i < SRCADDR_COUNT; i++)
		if (fds[i] >= 0)
			SAFE_CLOSE(fds[i]);

	if (childns >= 0)
		SAFE_CLOSE(childns);

	if (parentns >= 0)
		SAFE_CLOSE(parentns);
}

static struct tst_test test = {
	.test_all = run,
	.setup = setup,
	.cleanup = cleanup,
	.needs_kconfigs = (const char *[]) {
		"CONFIG_VETH",
		"CONFIG_USER_NS=y",
		"CONFIG_NET_NS=y",
		NULL
	},
	.save_restore = (const struct tst_path_val[]) {
		{"/proc/sys/user/max_user_namespaces", "1024", TST_SR_SKIP},
		{}
	},
	.tags = (const struct tst_tag[]) {
		{"linux-git", "b38e7819cae9"},
		{"CVE", "2020-25705"},
		{}
	}
};
