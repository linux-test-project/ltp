// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2020 Petr Vorel <pvorel@suse.cz>
 */

#include "config.h"
#include "tst_test.h"

#ifdef HAVE_LIBMNL

#include <string.h>

#include <libmnl/libmnl.h>
#include <linux/rtnetlink.h>
#include <net/if.h>
#include <netdb.h>
#include <netinet/in.h>

#include "tst_net.h"
#include "tst_safe_net.h"
#include "tst_safe_stdio.h"

#define IP_ADDR_DELIM ','
#define STR(x) #x
#define CHR2STR(x) STR(x)

static char *c_opt, *d_opt, *g_opt, *ipv6_opt, *p_opt, *r_opt;

static int family = AF_INET;
static int fd, num_loops, port;

static unsigned int is_ipv6, max, prefix;

static struct mnl_socket *nl;
static struct addrinfo hints;

struct iface {
	unsigned int index;
	struct iface *next;
	char iface[IFNAMSIZ];
};

struct ip_addr {
	struct addrinfo *ip;
	struct ip_addr *next;
	char ip_str[INET6_ADDRSTRLEN];
};

static struct ip_addr *dst, *gw, *rhost;
static struct iface *iface;
static unsigned int gw_len, iface_len, rhost_len;

void save_iface(void **data, const char *item)
{
	struct iface *n = SAFE_MALLOC(sizeof(*n));
	struct iface **list = (struct iface**)data;

	strncpy(n->iface, item, sizeof(n->iface));
	n->iface[sizeof(n->iface)-1] = '\0';

	n->index = if_nametoindex(item);
	if (!n->index)
		tst_brk(TBROK, "if_nametoindex failed, '%s' not found", item);
	n->next = *list;
	*list = n;
}

void save_ip(void **data, const char *item)
{
	struct ip_addr *n = SAFE_MALLOC(sizeof(*n));
	struct ip_addr **list = (struct ip_addr**)data;

	strncpy(n->ip_str, item, sizeof(n->ip_str));
	n->ip_str[sizeof(n->ip_str)-1] = '\0';

	SAFE_GETADDRINFO(item, p_opt, &hints, &n->ip);
	n->next = *list;
	*list = n;
}

int save_item(void **list, char *item, void (*callback)(void **, const char *))
{
	int len = 0;

	while ((item = strtok(item, CHR2STR(IP_ADDR_DELIM))) != NULL) {
		callback(list, item);
		item = NULL;
		len++;
	}

	return len;
}

static void setup(void)
{
	prefix = 24;
	if (ipv6_opt) {
		family = AF_INET6;
		is_ipv6 = 1;
		prefix = 64;
	}

	if (!c_opt)
		tst_brk(TBROK, "missing number of loops (-c num)");

	if (!d_opt)
		tst_brk(TBROK, "missing iface (-d iface)");

	if (!p_opt)
		tst_brk(TBROK, "missing rhost port (-p port)");

	if (!r_opt)
		tst_brk(TBROK, "missing rhost IP (-r IP)");

	if (tst_parse_int(p_opt, &port, 1, 65535))
		tst_brk(TBROK, "invalid rhost port '%s'", p_opt);

	if (tst_parse_int(c_opt, &num_loops, 1, INT_MAX)) {
		num_loops = INT_MAX;
		tst_res(TWARN, "invalid number of loops (-c %s), using: %d",
			c_opt, num_loops);
	}

	iface_len = save_item((void **)&iface, d_opt, save_iface);
	rhost_len = save_item((void **)&rhost, r_opt, save_ip);

	max = MAX(iface_len, rhost_len);
	if (iface_len > 1 && rhost_len > 1 && iface_len != max)
		tst_brk(TBROK, "-d specifies more NICs and -r more IPs, they need to have the same count");

	if (g_opt) {
		gw_len = save_item((void **)&gw, g_opt, save_ip);
		max = MAX(gw_len, max);

		if (gw_len > 1 && max > 1 && gw_len != max) {
			if (iface_len == max)
				tst_brk(TBROK, "-d specifies more NICs and -r more IPs, they need to have the same count");
			else
				tst_brk(TBROK, "-g and -r specify more IP, they need to have the same count");
		}
	}

	struct ip_addr *p_rhost = rhost;

	while (p_rhost) {
		char dst_str[INET6_ADDRSTRLEN];

		if (!strncpy(dst_str, p_rhost->ip_str, sizeof(dst_str)))
			tst_brk(TBROK, "failed copy IP '%s'", p_rhost->ip_str);
		dst_str[strlen(p_rhost->ip_str)-1] = '\0';

		if (!strcat(dst_str, "0"))
			tst_brk(TBROK, "strcat failed: '%s'", dst_str);

		save_ip((void **)&dst, dst_str);
		p_rhost = p_rhost->next;
	}

	fd = SAFE_SOCKET(family, SOCK_DGRAM, IPPROTO_UDP);

	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = family;
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_flags = 0;
	hints.ai_protocol = 0;
	hints.ai_addr = INADDR_ANY;
}

static void cleanup(void)
{
	if (fd > 0)
		close(fd);

	if (nl)
		mnl_socket_close(nl);
}

static void brk_on_route_error(const char *msg, int iface,
			       struct sockaddr *dst, struct sockaddr *gw, int type)
{
	char dst_str[INET6_ADDRSTRLEN], gw_str[INET6_ADDRSTRLEN];
	tst_sock_addr(dst, sizeof(dst), dst_str, sizeof(dst_str));
	if (gw)
		tst_sock_addr(gw, sizeof(gw), gw_str, sizeof(gw_str));

	tst_res(TINFO, "type: %s, iface: %d, dst: %s, gw: %s",
		type == RTM_NEWROUTE ? "RTM_NEWROUTE" : "RTM_DELROUTE",
		iface, dst_str, gw ? gw_str : "null");
	tst_brk(TBROK, "%s failed (errno=%d): %s", msg, errno, strerror(errno));
}

static void rtnl_route(int iface, struct addrinfo *dst, struct addrinfo *gw,
		       int type)
{
	struct mnl_socket *nl;
	char buf[MNL_SOCKET_BUFFER_SIZE];
	struct nlmsghdr *nlh;
	struct rtmsg *rtm;
	uint32_t seq, portid;
	struct in6_addr dst_in6, gw_in6;
	in_addr_t dst_ip, gw_ip;
	int ret;

	nlh = mnl_nlmsg_put_header(buf);
	nlh->nlmsg_type	= type;

	nlh->nlmsg_flags = NLM_F_ACK;
	if (type == RTM_NEWROUTE)
		nlh->nlmsg_flags |= NLM_F_REQUEST | NLM_F_CREATE | NLM_F_REPLACE;

	nlh->nlmsg_seq = seq = time(NULL);

	rtm = mnl_nlmsg_put_extra_header(nlh, sizeof(struct rtmsg));
	rtm->rtm_family = family;
	rtm->rtm_dst_len = prefix;
	rtm->rtm_src_len = 0;
	rtm->rtm_tos = 0;
	rtm->rtm_protocol = RTPROT_STATIC;
	rtm->rtm_table = RT_TABLE_MAIN;
	rtm->rtm_type = RTN_UNICAST;
	rtm->rtm_scope = gw ? RT_SCOPE_UNIVERSE : RT_SCOPE_LINK;
	rtm->rtm_flags = 0;

	if (is_ipv6) {
		dst_in6 = ((struct sockaddr_in6 *)dst->ai_addr)->sin6_addr;
		mnl_attr_put(nlh, RTA_DST, sizeof(struct in6_addr), &dst_in6);
	} else {
		dst_ip = ((struct sockaddr_in *)dst->ai_addr)->sin_addr.s_addr;
		mnl_attr_put_u32(nlh, RTA_DST, dst_ip);
	}

	mnl_attr_put_u32(nlh, RTA_OIF, iface);

	if (gw) {
		if (is_ipv6) {
			gw_in6 = ((struct sockaddr_in6 *)gw->ai_addr)->sin6_addr;
			mnl_attr_put(nlh, RTA_GATEWAY, sizeof(struct in6_addr), &gw_in6);
		} else {
			gw_ip = ((struct sockaddr_in *)gw->ai_addr)->sin_addr.s_addr;
			mnl_attr_put_u32(nlh, RTA_GATEWAY, gw_ip);
		}
	}

	nl = mnl_socket_open(NETLINK_ROUTE);
	if (nl == NULL)
		brk_on_route_error("mnl_socket_open", iface, dst->ai_addr, gw ?
				   gw->ai_addr : NULL, type);

	if (mnl_socket_bind(nl, 0, MNL_SOCKET_AUTOPID) < 0)
		brk_on_route_error("mnl_socket_bind", iface, dst->ai_addr, gw ?
				   gw->ai_addr : NULL, type);

	portid = mnl_socket_get_portid(nl);

	if (mnl_socket_sendto(nl, nlh, nlh->nlmsg_len) < 0)
		brk_on_route_error("mnl_socket_sendto", iface, dst->ai_addr, gw
				   ? gw->ai_addr : NULL, type);

	ret = mnl_socket_recvfrom(nl, buf, sizeof(buf));
	if (ret < 0)
		brk_on_route_error("mnl_socket_recvfrom", iface, dst->ai_addr,
				   gw ? gw->ai_addr : NULL, type);

	ret = mnl_cb_run(buf, ret, seq, portid, NULL, NULL);
	if (ret < 0)
		brk_on_route_error("mnl_cb_run", iface, dst->ai_addr, gw ?
				   gw->ai_addr : NULL, type);

	mnl_socket_close(nl);
}

static void send_udp(struct addrinfo *rhost_addrinfo)
{
	const char *msg = "foo";

	SAFE_SENDTO(1, fd, msg, sizeof(msg), MSG_CONFIRM,
		rhost_addrinfo->ai_addr, rhost_addrinfo->ai_addrlen);
}

static void run(void)
{
	int i;

	tst_res(TINFO, "adding and deleting route %d times", num_loops);

	struct ip_addr *p_dst = dst, *p_gw = gw, *p_rhost = rhost;
	struct iface *p_iface = iface;

	for (i = 0; i < num_loops; i++) {
		rtnl_route(p_iface->index, p_dst->ip, gw ? p_gw->ip : NULL,
			   RTM_NEWROUTE);
		send_udp(p_rhost->ip);
		rtnl_route(p_iface->index, p_dst->ip, gw ? p_gw->ip : NULL,
			   RTM_DELROUTE);

		if (gw)
			p_gw = p_gw->next ?: gw;
		p_dst = p_dst->next ?: dst;
		p_iface = p_iface->next ?: iface;
		p_rhost = p_rhost->next ?: rhost;
	}

	tst_res(TPASS, "routes created and deleted");
}

static struct tst_test test = {
	.test_all = run,
	.needs_root = 1,
	.setup = setup,
	.cleanup = cleanup,
	.options = (struct tst_option[]) {
		{"6", &ipv6_opt, "Use IPv6 (default is IPv4)"},
		{"c:", &c_opt, "Num loops (mandatory)"},
		{"d:", &d_opt, "Interface to work on (mandatory)"},
		{"g:", &g_opt, "Gateway IP"},
		{"p:", &p_opt, "Rhost port (mandatory)"},
		{"r:", &r_opt, "Rhost IP (mandatory)\n\n-g, -r IP parameter can contain more IP, separated by "
			CHR2STR(IP_ADDR_DELIM)},
		{}
	},
};
#else
	TST_TEST_TCONF("libmnl library and headers are required");
#endif /* HAVE_LIBMNL */
