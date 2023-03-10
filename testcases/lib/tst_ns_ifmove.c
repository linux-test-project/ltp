// SPDX-License-Identifier: GPL-2.0-or-later
/* Copyright (c) 2015 Red Hat, Inc.
 * Copyright (c) Linux Test Project, 2015-2022
 * Copyright (c) 2023 Petr Vorel <pvorel@suse.cz>
 * Written by Matus Marhefka <mmarhefk@redhat.com>
 */

/*\
 * [Description]
 *
 * Moves a network interface to the namespace of a process specified by a PID.
 */

#include "config.h"

#define TST_NO_DEFAULT_MAIN
#include "tst_test.h"
#include "tst_safe_macros.h"
#include "tst_safe_net.h"

#include <linux/if.h>
#include <linux/rtnetlink.h>
#include <net/ethernet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef HAVE_DECL_IFLA_NET_NS_PID

static struct {
	struct nlmsghdr nh;
	struct ifinfomsg ifi;
	char attrbuf[512];
} req;


static int get_intf_index_from_name(const char *intf_name)
{
	struct ifreq ifr;
	int sock_fd;

	memset(&ifr, 0, sizeof(ifr));
	strncpy(ifr.ifr_name, intf_name, sizeof(ifr.ifr_name) - 1);
	ifr.ifr_name[sizeof(ifr.ifr_name)-1] = '\0';

	sock_fd = SAFE_SOCKET(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL));

	/* interface index */
	SAFE_IOCTL(sock_fd, SIOCGIFINDEX, &ifr);
	SAFE_CLOSE(sock_fd);

	return ifr.ifr_ifindex;
}

int main(int argc, char **argv)
{
	struct rtattr *rta;
	int intf_index, pid, rtnetlink_socket;

	if (argc != 3) {
		printf("tst_ns_ifmove <INTERFACE_NAME> <NAMESPACE_PID>\n");
		return 1;
	}

	intf_index = get_intf_index_from_name(argv[1]);
	pid = atoi(argv[2]);
	rtnetlink_socket = SAFE_SOCKET(AF_NETLINK, SOCK_DGRAM, NETLINK_ROUTE);

	memset(&req, 0, sizeof(req));
	req.nh.nlmsg_len = NLMSG_LENGTH(sizeof(struct ifinfomsg));
	req.nh.nlmsg_flags = NLM_F_REQUEST;
	req.nh.nlmsg_type = RTM_NEWLINK;
	req.ifi.ifi_family = AF_UNSPEC;
	req.ifi.ifi_index = intf_index;
	req.ifi.ifi_change = 0xffffffff;
	rta = (struct rtattr *)(((char *) &req) +
		NLMSG_ALIGN(req.nh.nlmsg_len));
	rta->rta_type = IFLA_NET_NS_PID;
	rta->rta_len = RTA_LENGTH(sizeof(int));
	req.nh.nlmsg_len = NLMSG_ALIGN(req.nh.nlmsg_len) +
		RTA_LENGTH(sizeof(pid));
	memcpy(RTA_DATA(rta), &pid, sizeof(pid));

	SAFE_SEND(1, rtnetlink_socket, &req, req.nh.nlmsg_len, 0);
	SAFE_CLOSE(rtnetlink_socket);

	return 0;
}

#else
	TST_TEST_TCONF("IFLA_NET_NS_PID not defined in linux/if_link.h");
#endif
