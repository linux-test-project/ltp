// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2017 Petr Vorel <pvorel@suse.cz>
 * Copyright (c) 1997-2015 Red Hat, Inc. All rights reserved.
 * Copyright (c) 2011-2013 Rich Felker, et al.
 */

#include <arpa/inet.h>
#include <arpa/nameser.h>
#include <assert.h>
#include <errno.h>
#include <linux/rtnetlink.h>
#include <net/if.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdarg.h>


#define TST_NO_DEFAULT_MAIN
#include "tst_test.h"

#include "tst_net.h"
#include "tst_private.h"

#define BASE_IPV4_PREFIX 8
#define BASE_IPV6_PREFIX 16

#define MAX_IPV4_PREFIX 32
#define MAX_IPV6_PREFIX 128

#define DEFAULT_IPV4_UNUSED_PART1 10
#define DEFAULT_IPV6_UNUSED_PART1 0xfd

#define DEFAULT_IPV4_UNUSED_PART2 23
#define DEFAULT_IPV6_UNUSED_PART2 0x23

struct ltp_net_variables {
	char *ipv4_lbroadcast;
	char *ipv4_rbroadcast;
	char *ipv4_lnetmask;
	char *ipv4_rnetmask;
	char *ipv4_lnetwork;
	char *ipv4_rnetwork;
	char *lhost_ipv4_host;
	char *rhost_ipv4_host;
	char *ipv6_lnetmask;
	char *ipv6_rnetmask;
	char *ipv6_lnetwork;
	char *ipv6_rnetwork;
	char *lhost_ipv6_host;
	char *rhost_ipv6_host;
	char *ipv4_net16_unused;
	char *ipv6_net32_unused;
};
static struct ltp_net_variables vars;

static void usage(const char *cmd)
{
	fprintf(stderr, "USAGE:\n"
		"%s IP_LHOST[/PREFIX] IP_RHOST[/PREFIX]\n"
		"%s -h\n\n"
		"Exported variables:\n"
		"IPV4_LBROADCAST: IPv4 broadcast of the local host\n"
		"IPV4_RBROADCAST: IPv4 broadcast of the remote host\n"
		"IPV4_LNETMASK: IPv4 netmask of the local host\n"
		"IPV4_RNETMASK: IPv4 netmask of the remote host\n"
		"IPV4_LNETWORK: IPv4 network part of IPV4_LHOST\n"
		"IPV4_RNETWORK: IPv4 network part of IPV4_RHOST\n"
		"LHOST_IPV4_HOST IPv4 host part of IPV4_LHOST\n"
		"RHOST_IPV4_HOST IPv4 host part of IPV4_RHOST\n"
		"IPV4_NET16_UNUSED: IPv4 16 bit unused subnet\n"
		"IPV6_LNETMASK: IPv6 netmask of the local host\n"
		"IPV6_RNETMASK: IPv6 netmask of the remote host\n"
		"IPV6_LNETWORK: IPv6 network part of IPV6_LHOST\n"
		"IPV6_RNETWORK: IPv6 network part of IPV6_LHOST\n"
		"LHOST_IPV6_HOST: IPv6 unique part of IPV6_LHOST\n"
		"RHOST_IPV6_HOST: IPv6 unique part of IPV6_RHOST\n"
		"IPV6_NET32_UNUSED: IPv6 32 bit unused subnet\n\n"
		"NOTE: Prefixes, ifaces and lhosts are expected to be set by tst_net_iface_prefix.\n"
		"OPTIONS:\n"
		"-h this help\n",
		cmd, cmd);
}

/*
 * Function prefix2mask is from ipcalc project, ipcalc.c.
 */
static struct in_addr prefix2mask(unsigned int prefix)
{
	struct in_addr mask;

	memset(&mask, 0x0, sizeof(mask));

	if (prefix)
		mask.s_addr = htonl(~((1 << (32 - prefix)) - 1));
	else
		mask.s_addr = htonl(0);

	return mask;
}

/*
 * Function calc_network is based on ipcalc project,
 * calc_network/ipcalc.c.
 */
static struct in_addr calc_network(const struct in_addr *ip,
	struct in_addr *mask)
{
	struct in_addr network;

	memset(&network, 0, sizeof(network));

	network.s_addr = ip->s_addr & mask->s_addr;
	return network;
}

static int is_in_subnet_ipv4(const struct in_addr *network,
	const struct in_addr *mask, const struct in_addr *ip)
{
	return (ip->s_addr & mask->s_addr) ==
		(network->s_addr & mask->s_addr);
}

static int is_in_subnet_ipv6(const struct in6_addr *network,
	const struct in6_addr *mask, const struct in6_addr *ip6)
{
	unsigned int i;

	for (i = 0; i < sizeof(struct in6_addr) / sizeof(int); i++) {
		if (((((int *) ip6)[i] & ((int *) mask)[i])) !=
			(((int *) network)[i] & ((int *) mask)[i]))
			return 0;
	}
	return 1;
}

/*
 * Function get_ipv4_netmask uses code from calc_network, which is from
 * ipcalc project, ipcalc.c.
 */
static char *get_ipv4_netmask(unsigned int prefix)
{
	char buf[INET_ADDRSTRLEN];
	struct in_addr mask = prefix2mask(prefix);

	if (prefix > MAX_IPV4_PREFIX)
		return NULL;

	if (!inet_ntop(AF_INET, &mask, buf, sizeof(buf)))
		tst_brk_comment("error calculating IPv4 address");

	return strdup(buf);
}

/*
 * Function get_ipv4_netmask uses code from ipv6_prefix_to_mask and
 * ipv6_mask_to_str, which are from ipcalc project, ipcalc.c.
 */
static char *get_ipv6_netmask(unsigned int prefix)
{
	struct in6_addr in6;
	char buf[128];
	int i, j;

	if (prefix > MAX_IPV6_PREFIX)
		return NULL;

	memset(&in6, 0x0, sizeof(in6));
	for (i = prefix, j = 0; i > 0; i -= 8, j++) {
		if (i >= 8)
			in6.s6_addr[j] = 0xff;
		else
			in6.s6_addr[j] = (unsigned long)(0xffU << (8 - i));
	}

	if (!inet_ntop(AF_INET6, &in6, buf, sizeof(buf)))
		tst_brk_comment("error calculating IPv6 address");

	return strdup(buf);
}

/*
 * Function get_ipv4_broadcast uses code from calc_broadcast, which is from
 * ipcalc project, ipcalc.c.
 */
static char *get_ipv4_broadcast(struct in_addr ip, unsigned int prefix)
{
	struct in_addr mask = prefix2mask(prefix);
	struct in_addr broadcast;
	char buf[INET_ADDRSTRLEN];

	memset(&broadcast, 0, sizeof(broadcast));
	broadcast.s_addr = (ip.s_addr & mask.s_addr) | ~mask.s_addr;

	if (!inet_ntop(AF_INET, &broadcast, buf, sizeof(buf)))
		tst_brk_comment("error calculating IPv4 address");

	return strdup(buf);
}

/*
 * For unused network we use
 * DEFAULT_IPV4_UNUSED_PART1:DEFAULT_IPV4_UNUSED_PART2 or
 * {DEFAULT_IPV4_UNUSED_PART1}.XY, when there is a collision with IP.
 */
static char *get_ipv4_net16_unused(const struct in_addr *ip,
	unsigned int prefix)
{
	struct in_addr mask, network;
	char buf[132], net_unused[128];

	mask = prefix2mask(prefix);
	network = calc_network(ip, &mask);

	sprintf(net_unused, "%d.%d", DEFAULT_IPV4_UNUSED_PART1,
			DEFAULT_IPV4_UNUSED_PART2);
	sprintf(buf, "%s.0.0", net_unused);

	tst_get_in_addr(buf, &network);

	if (!is_in_subnet_ipv4(ip, &mask, &network))
		return strdup(net_unused);

	srand(time(NULL));

	/* try to randomize second group */
	sprintf(net_unused, "%d.%d", DEFAULT_IPV4_UNUSED_PART1,
		(rand() % 128) + (((ip->s_addr >> 8) & 0xff) < 128 ? 128 : 0));
	sprintf(buf, "%s.0.0", net_unused);

	tst_get_in_addr(buf, &network);

	if (!is_in_subnet_ipv4(ip, &mask, &network))
		return strdup(net_unused);

	/* try to randomize first group */
	sprintf(net_unused, "%d.%d", (rand() % 128) + (((ip->s_addr) & 0xff)
			< 128 ? 128 : 0), DEFAULT_IPV4_UNUSED_PART2);
	sprintf(buf, "%s.0.0", net_unused);

	tst_get_in_addr(buf, &network);

	if (!is_in_subnet_ipv4(ip, &mask, &network))
		return strdup(net_unused);

	return NULL;
}

/*
 * Function get_ipv6_net32_unused is inspired by ipcalc project,
 * get_ipv6_info/ipcalc.c.
 *
 * For unused network we use DEFAULT_IPV6_UNUSED_PART1:DEFAULT_IPV6_UNUSED_PART2
 * if no collision with existing IP range.
 * Otherwise we try to use
 * {DEFAULT_IPV6_UNUSED_PART1}XY:DEFAULT_IPV6_UNUSED_PART2 or
 * XY:DEFAULT_IPV6_UNUSED_PART2.
 */
static char *get_ipv6_net32_unused(const struct in6_addr *ip6,
	unsigned int prefix)
{
	int i, j;
	struct in6_addr mask, network;
	char buf[130], net_unused[128];

	memset(&mask, 0x0, sizeof(mask));

	if (prefix > 128)
		return NULL;


	for (i = prefix, j = 0; i > 0; i -= 8, j++) {
		if (i >= 8)
			mask.s6_addr[j] = 0xff;
		else
			mask.s6_addr[j] = (unsigned long)(0xffU << (8 - i));
	}

	sprintf(net_unused, "%x:%x", 256 * DEFAULT_IPV6_UNUSED_PART1,
			DEFAULT_IPV6_UNUSED_PART2);
	sprintf(buf, "%s::", net_unused);

	tst_get_in6_addr(buf, &network);

	if (!is_in_subnet_ipv6(ip6, &mask, &network))
		return strdup(net_unused);

	srand(time(NULL));

	/* try to randomize second group */
	sprintf(net_unused, "%x:%x", 256 * DEFAULT_IPV6_UNUSED_PART1 +
			(rand() % 128) + (ip6->s6_addr[1] < 128 ? 128 : 0),
			DEFAULT_IPV6_UNUSED_PART2);
	sprintf(buf, "%s::", net_unused);

	tst_get_in6_addr(buf, &network);

	if (!is_in_subnet_ipv6(ip6, &mask, &network))
		return strdup(net_unused);

	/* try to randomize first group */
	sprintf(net_unused, "%x:%x",
			256 * (rand() % 128) + (256 * ip6->s6_addr[0] < 128 ?
			128 : 0), DEFAULT_IPV6_UNUSED_PART2);
	sprintf(buf, "%s::", net_unused);

	tst_get_in6_addr(buf, &network);

	if (!is_in_subnet_ipv6(ip6, &mask, &network))
		return strdup(net_unused);

	return NULL;
}

/*
 * Function get_ipv6_network is based on musl libc project,
 * inet_ntop/inet_ntop.c.
 */
static char *get_ipv6_network(const unsigned char *a0, unsigned int prefix)
{
	const unsigned char *a = a0;
	unsigned int i, j, max, best, border = 0;
	char buf[100];
	char ret[100];
	char tmp[100];
	char *p_ret = ret;
	char *p_tmp = tmp;
	size_t offset;

	if (prefix > MAX_IPV6_PREFIX)
		return NULL;

	if (prefix == MAX_IPV6_PREFIX)
		return strdup("\0");

	snprintf(buf, sizeof(buf),
		"%x:%x:%x:%x:%x:%x:%x:%x",
		256 * a[0] + a[1], 256 * a[2] + a[3],
		256 * a[4] + a[5], 256 * a[6] + a[7],
		256 * a[8] + a[9], 256 * a[10] + a[11],
		256 * a[12] + a[13], 256 * a[14] + a[15]);

	for (i = 0; i < 8; i++) {
		if (i < prefix >> 4) {
			border += sprintf(p_tmp, "%x", 256 * a[2 * i] +
				a[2 * i + 1]);
			if (i > 0)
				border++;
		}

		if (i >= prefix >> 4)
			break;

		/* ':' only if no leading in host or ending in net */
		if (i > 0)
			*p_ret++ = ':';

		offset = sprintf(p_ret, "%x", 256 * a[2 * i] + a[2 * i + 1]);
		p_ret += offset;
	}

	*p_ret = '\0';

	/* Find longest /(^0|:)[:0]{2,}/ */
	for (i = best = 0, max = 2; buf[i]; i++) {
		if (i && buf[i] != ':')
			continue;
		j = strspn(buf + i, ":0");

		if (j > max)
			best = i, max = j;
	}

	size_t length = strlen(ret);
	size_t best_end = best + max - 1;

	if (max > 2 && best < border) {
		p_ret = ret;
		/* Replace longest /(^0|:)[:0]{2,}/ with "::" */
		if (best == 0 && best_end >= border) {
			/* zeros in whole net part or continue to host */
			ret[0] = ':';
			ret[1] = '\0';
		} else if (best == 0 && best_end < border) {
			/* zeros on beginning, not whole part */
			ret[0] = ':';
			memmove(p_ret + 1, p_ret + best_end, border - best_end
				+ 1);
		} else if (best > 0 && best_end >= border) {
			/*
			 * zeros not from beginning to border or continue to
			 * host
			 */
			ret[best] = ':';
			ret[best + 1] = '\0';
		} else {
			/* zeros somewhere in the middle */
			ret[best] = ':';
			memmove(p_ret + best + 1, p_ret + best_end,
					border - best + 1);
		}
	}

	if (length < INET6_ADDRSTRLEN)
		return strdup(ret);

	return NULL;
}

/*
 * Strip host part from ip address.
 */
static char *get_host_from_ip(const char *ip, const char *net)
{
	if (ip == NULL || net == NULL)
		return NULL;

	char *result = strstr(ip, net);

	if (!result || result != ip)
		return NULL;

	char *buf = strdup(ip);
	unsigned int index = strlen(net);
	int len;

	/* prefix < 8 (IPv4) or 128 (IPv6) */
	if (index == strlen(ip))
		return strdup("\0");

	/* prefix > 0 && prefix < 32 (IPv4) or 128 (IPv6) */
	if (index > 0 && index < strlen(ip)) {
		len = strlen(ip) - index - 1;
		assert(ip[index] == ':' || ip[index] == '.');
		memmove(buf, buf + index + 1, len);
		buf[len] = '\0';
	}

	return buf;
}

static void check_prefix_range(unsigned int prefix, int is_ipv6, int is_lhost)
{
	unsigned int base_prefix = is_ipv6 ? BASE_IPV6_PREFIX :
		BASE_IPV4_PREFIX;
	unsigned int max_prefix = is_ipv6 ? MAX_IPV6_PREFIX : MAX_IPV4_PREFIX;

	if (prefix < base_prefix || (is_ipv6 && prefix == 128) ||
		(!is_ipv6 && prefix == 32))
		tst_res_comment(TWARN,
			"prefix %d for %s will be unsuitable for some stress tests which need %s variable. To avoid this use prefix >= %d and prefix < %d.",
			prefix, is_ipv6 ?  "IPv6" : "IPv4",
			is_ipv6 ?
				(is_lhost ? "IPV6_LNETWORK" : "IPV6_RNETWORK") :
				(is_lhost ? "IPV4_LNETWORK" : "IPV4_RNETWORK"),
			base_prefix, max_prefix);
}

static char *get_ipv4_network(int ip, unsigned int prefix)
{
	char buf[INET_ADDRSTRLEN];
	char *p_buf = buf;
	unsigned char byte;
	unsigned int i;

	ip = htonl(ip);

	if (prefix > MAX_IPV4_PREFIX)
		return NULL;

	if (prefix == MAX_IPV4_PREFIX)
		return strdup("\0");

	prefix &= MAX_IPV4_PREFIX - 8;

	for (i = prefix; i > 0; i -= 8) {
		byte = (ip >> i) & 0xff;
		sprintf(p_buf, i < prefix ? ".%d" : "%d", byte);
		p_buf += strlen(p_buf);
	}

	return strdup(buf);
}

/*
 * Round down prefix.
 */
static int round_down_prefix(unsigned int prefix, int is_ipv6)
{
	unsigned int base_prefix = is_ipv6 ? BASE_IPV6_PREFIX :
		BASE_IPV4_PREFIX;

	return prefix / base_prefix * base_prefix;
}

static void get_ipv4_info(const char *lip_str, const char *rip_str, int lprefix,
	int rprefix)
{
	struct in_addr lip, rip;
	int lprefix_round, rprefix_round;

	lprefix_round = round_down_prefix(lprefix, 0);
	rprefix_round = round_down_prefix(rprefix, 0);

	tst_get_in_addr(lip_str, &lip);
	tst_get_in_addr(rip_str, &rip);

	vars.ipv4_lbroadcast = get_ipv4_broadcast(lip, lprefix);
	vars.ipv4_rbroadcast = get_ipv4_broadcast(rip, rprefix);

	vars.ipv4_lnetmask = get_ipv4_netmask(lprefix);
	vars.ipv4_rnetmask = get_ipv4_netmask(rprefix);

	vars.ipv4_lnetwork = get_ipv4_network(lip.s_addr, lprefix_round);
	vars.ipv4_rnetwork = get_ipv4_network(rip.s_addr, rprefix_round);

	vars.lhost_ipv4_host = get_host_from_ip(lip_str, vars.ipv4_lnetwork);
	vars.rhost_ipv4_host = get_host_from_ip(rip_str, vars.ipv4_rnetwork);

	vars.ipv4_net16_unused = get_ipv4_net16_unused(&lip, lprefix_round);
}

static void get_ipv6_info(const char *lip_str, const char *rip_str,
	int lprefix, int rprefix)
{
	struct in6_addr lip, rip;
	int lprefix_round, rprefix_round;

	lprefix_round = round_down_prefix(lprefix, 1);
	rprefix_round = round_down_prefix(rprefix, 1);

	tst_get_in6_addr(lip_str, &lip);
	tst_get_in6_addr(rip_str, &rip);

	vars.ipv6_lnetmask = get_ipv6_netmask(lprefix);
	vars.ipv6_rnetmask = get_ipv6_netmask(rprefix);

	vars.ipv6_lnetwork = get_ipv6_network(lip.s6_addr, lprefix_round);
	vars.ipv6_rnetwork = get_ipv6_network(rip.s6_addr, rprefix_round);

	vars.lhost_ipv6_host = get_host_from_ip(lip_str, vars.ipv6_lnetwork);
	vars.rhost_ipv6_host = get_host_from_ip(rip_str, vars.ipv6_rnetwork);

	vars.ipv6_net32_unused = get_ipv6_net32_unused(&lip, lprefix_round);
}

static void print_vars(int is_ipv6)
{
	if (is_ipv6) {
		tst_print_svar("IPV6_LNETMASK", vars.ipv6_lnetmask);
		tst_print_svar_change("IPV6_RNETMASK", vars.ipv6_rnetmask);
		tst_print_svar("IPV6_LNETWORK", vars.ipv6_lnetwork);
		tst_print_svar("IPV6_RNETWORK", vars.ipv6_rnetwork);
		tst_print_svar("LHOST_IPV6_HOST", vars.lhost_ipv6_host);
		tst_print_svar("RHOST_IPV6_HOST", vars.rhost_ipv6_host);
		tst_print_svar("IPV6_NET32_UNUSED", vars.ipv6_net32_unused);
	} else {
		tst_print_svar("IPV4_LBROADCAST", vars.ipv4_lbroadcast);
		tst_print_svar_change("IPV4_RBROADCAST", vars.ipv4_rbroadcast);
		tst_print_svar("IPV4_LNETMASK", vars.ipv4_lnetmask);
		tst_print_svar_change("IPV4_RNETMASK", vars.ipv4_rnetmask);
		tst_print_svar("IPV4_LNETWORK", vars.ipv4_lnetwork);
		tst_print_svar("IPV4_RNETWORK", vars.ipv4_rnetwork);
		tst_print_svar("LHOST_IPV4_HOST", vars.lhost_ipv4_host);
		tst_print_svar("RHOST_IPV4_HOST", vars.rhost_ipv4_host);
		tst_print_svar("IPV4_NET16_UNUSED", vars.ipv4_net16_unused);
	}
}

int main(int argc, char *argv[])
{
	char *lip_str = NULL, *rip_str = NULL;
	int is_ipv6, lprefix, rprefix, tmp;
	struct in_addr ip;
	struct in6_addr ip6;

	int is_usage = argc > 1 && (!strcmp(argv[1], "-h") ||
		!strcmp(argv[1], "--help"));
	if (argc < 3 || is_usage) {
		usage(argv[0]);
		exit(is_usage ? EXIT_SUCCESS : EXIT_FAILURE);
	}

	lip_str = argv[1];
	rip_str = argv[2];

	is_ipv6 = !!strchr(lip_str, ':');
	lprefix = tst_get_prefix(lip_str, is_ipv6);
	rprefix = tst_get_prefix(rip_str, is_ipv6);

	if (is_ipv6)
		tst_get_in6_addr(lip_str, &ip6);
	else
		tst_get_in_addr(lip_str, &ip);

	tmp = !!strchr(rip_str, ':');
	if (tmp)
		tst_get_in6_addr(rip_str, &ip6);
	else
		tst_get_in_addr(rip_str, &ip);

	if (is_ipv6 != tmp)
		tst_brk_comment("mixed IPv4 and IPv6 addresses ('%s', '%s')",
				lip_str, rip_str);

	check_prefix_range(lprefix, is_ipv6, 1);
	check_prefix_range(rprefix, is_ipv6, 0);

	if (!strcmp(lip_str, rip_str))
		tst_brk_comment("IP addresses cannot be the same ('%s', '%s')",
				lip_str, rip_str);

	if (is_ipv6)
		get_ipv6_info(lip_str, rip_str, lprefix, rprefix);
	else
		get_ipv4_info(lip_str, rip_str, lprefix, rprefix);

	print_vars(is_ipv6);

	exit(EXIT_SUCCESS);
}
