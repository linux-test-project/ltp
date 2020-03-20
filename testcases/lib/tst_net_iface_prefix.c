// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2017 Petr Vorel <pvorel@suse.cz>
 */

#include <sys/socket.h>
#include <linux/rtnetlink.h>
#include <net/if.h>
#include <stdio.h>
#include <stdlib.h>

#define TST_NO_DEFAULT_MAIN
#include "tst_test.h"

#include "tst_net.h"
#include "tst_private.h"

static char *iface;
static int prefix;

static void usage(const char *cmd)
{
	fprintf(stderr, "USAGE:\n"
		"%s IP_LHOST[/PREFIX]\n"
		"%s -r IP_RHOST[/PREFIX]\n"
		"%s -h\n\n"
		"Set prefix and interface name for given IP.\n"
		"Prefix and interface are found from kernel exported info (rtnetlink).\n\n"
		"EXPORTED VARIABLES:\n"
		"Export one of the following variables:\n"
		"IPV4_LPREFIX: IPv4 prefix for IPV4_LNETWORK\n"
		"IPV4_RPREFIX: IPv4 prefix for IPV4_RNETWORK\n"
		"IPV6_LPREFIX: IPv6 prefix for IPV6_LNETWORK\n"
		"IPV6_RPREFIX: IPv6 prefix for IPV6_RNETWORK\n"
		"Export one of the following variables (if found):\n"
		"LHOST_IFACES: iface name of the local host\n"
		"RHOST_IFACES: iface name of the remote host\n\n"
		"PARAMS:\n"
		"-h this help\n"
		"-r export remote environment variables\n",
		cmd, cmd, cmd);
}

static int read_iface_prefix(const char *ip_str, int is_ipv6)
{
	uint8_t family = is_ipv6 ? AF_INET6 : AF_INET;

	char buf[16384];
	unsigned int len;

	struct {
		struct nlmsghdr nlhdr;
		struct ifaddrmsg addrmsg;
	} msg;

	struct nlmsghdr *retmsg;

	int sock = SAFE_SOCKET(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE);

	memset(&msg, 0, sizeof(msg));
	msg.nlhdr.nlmsg_len = NLMSG_LENGTH(sizeof(struct ifaddrmsg));
	msg.nlhdr.nlmsg_flags = NLM_F_REQUEST | NLM_F_ROOT;
	msg.nlhdr.nlmsg_type = RTM_GETADDR;
	msg.addrmsg.ifa_family = family;

	SAFE_SEND(1, sock, &msg, msg.nlhdr.nlmsg_len, 0);
	len = recv(sock, buf, sizeof(buf), 0);
	retmsg = (struct nlmsghdr *)buf;

	while NLMSG_OK(retmsg, len) {
		char ifname[IFNAMSIZ];
		struct ifaddrmsg *retaddr;
		struct rtattr *retrta;
		char pradd[128];
		int attlen;

		retaddr = (struct ifaddrmsg *)NLMSG_DATA(retmsg);
		retrta = (struct rtattr *)IFA_RTA(retaddr);
		attlen = IFA_PAYLOAD(retmsg);

		while RTA_OK(retrta, attlen) {
			if (retrta->rta_type == IFA_ADDRESS) {
				inet_ntop(family, RTA_DATA(retrta), pradd,
					  sizeof(pradd));

				if_indextoname(retaddr->ifa_index, ifname);

				if (!strcmp(pradd, ip_str)) {
					prefix = retaddr->ifa_prefixlen;
					iface = strdup(ifname);
					return 0;
				}
			}
			retrta = RTA_NEXT(retrta, attlen);
		}
		retmsg = NLMSG_NEXT(retmsg, len);
	}

	return -1;
}

static void print_ivar(const char *name, unsigned int val)
{
	printf("export %s=%d\n", name, val);
}

int main(int argc, char *argv[])
{
	char *ip_str = NULL, *prefix_str = NULL;
	int is_ipv6, is_rhost = 0;
	struct in_addr ip;
	struct in6_addr ip6;

	int is_usage = argc > 1 && (!strcmp(argv[1], "-h") ||
		!strcmp(argv[1], "--help"));
	if (argc < 2 || is_usage) {
		usage(argv[0]);
		exit(is_usage ? EXIT_SUCCESS : EXIT_FAILURE);
	}
	if (!strcmp(argv[1], "-r"))
		is_rhost = 1;

	ip_str = argv[is_rhost ? 2 : 1];
	is_ipv6 = !!strchr(ip_str, ':');

	prefix_str = strchr(ip_str, '/');
	if (prefix_str) {
		prefix = tst_get_prefix(ip_str, is_ipv6);
		tst_res_comment(TINFO,
			"IP address '%s' contains prefix %d, using it and don't search for iface.\n",
			ip_str, prefix);
	} else if (read_iface_prefix(ip_str, is_ipv6)) {
		tst_res_comment(TINFO,
			"prefix and interface not found for '%s'.\n", ip_str);
		exit(EXIT_SUCCESS);
	}

	/* checks for validity of IP string */
	if (is_ipv6)
		tst_get_in6_addr(ip_str, &ip6);
	else
		tst_get_in_addr(ip_str, &ip);

	tst_print_svar_change(is_rhost ? "RHOST_IFACES" : "LHOST_IFACES",
		iface);
	if (is_ipv6)
		print_ivar(is_rhost ? "IPV6_RPREFIX" : "IPV6_LPREFIX", prefix);
	else
		print_ivar(is_rhost ? "IPV4_RPREFIX" : "IPV4_LPREFIX", prefix);

	exit(EXIT_SUCCESS);
}
