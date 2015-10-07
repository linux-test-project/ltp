/* Copyright (c) 2015 Red Hat, Inc.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of version 2 the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Written by Matus Marhefka <mmarhefk@redhat.com>
 *
 ***********************************************************************
 * Moves a network interface to the namespace of a process specified by a PID.
 *
 */

#define _GNU_SOURCE
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <asm/types.h>
#include <sys/socket.h>
#include <linux/rtnetlink.h>
#include <sys/ioctl.h>
#include <linux/if.h>
#include <net/ethernet.h>
#include <arpa/inet.h>
#include "test.h"

#include "config.h"

char *TCID = "ns_ifmove";

#if HAVE_DECL_IFLA_NET_NS_PID

struct {
	struct nlmsghdr nh;
	struct ifinfomsg ifi;
	char attrbuf[512];
} req;


int get_intf_index_from_name(const char *intf_name)
{
	struct ifreq ifr;
	int sock_fd;

	memset(&ifr, 0, sizeof(ifr));
	strncpy(ifr.ifr_name, intf_name, sizeof(ifr.ifr_name) - 1);
	ifr.ifr_name[sizeof(ifr.ifr_name)-1] = '\0';

	sock_fd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
	if (sock_fd == -1) {
		tst_resm(TINFO | TERRNO, "socket");
		return -1;
	}

	/* gets interface index */
	if (ioctl(sock_fd, SIOCGIFINDEX, &ifr) == -1) {
		tst_resm(TINFO | TERRNO, "ioctl");
		close(sock_fd);
		return -1;
	}

	close(sock_fd);
	return ifr.ifr_ifindex;
}

/*
 * ./ns_ifmove <INTERFACE_NAME> <NAMESPACE_PID>
 */
int main(int argc, char **argv)
{
	struct rtattr *rta;
	int intf_index, pid, rtnetlink_socket;

	if (argc != 3) {
		tst_resm(TINFO, "%s <INTERFACE_NAME> <NAMESPACE_PID>\n",
			 argv[0]);
		return 1;
	}

	intf_index = get_intf_index_from_name(argv[1]);
	if (intf_index == -1) {
		tst_resm(TINFO , "unable to get interface index");
		return 1;
	}

	pid = atoi(argv[2]);

	rtnetlink_socket = socket(AF_NETLINK, SOCK_DGRAM, NETLINK_ROUTE);
	if (rtnetlink_socket == -1) {
		tst_resm(TINFO | TERRNO, "socket");
		return 1;
	}

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

	if (send(rtnetlink_socket, &req, req.nh.nlmsg_len, 0) == -1) {
		tst_resm(TINFO | TERRNO, "send");
		return 1;
	}

	close(rtnetlink_socket);
	return 0;
}

#else

int main(void)
{
	tst_brkm(TCONF, NULL, "IFLA_NET_NS_PID not defined in linux/if_link.h");
}

#endif
