// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2021 Linux Test Project
 */

#include <asm/types.h>
#include <linux/veth.h>
#include <sys/socket.h>
#include <net/if.h>
#include "lapi/rtnetlink.h"

#define TST_NO_DEFAULT_MAIN
#include "tst_test.h"
#include "tst_rtnetlink.h"
#include "tst_netdevice.h"

static struct tst_rtnl_context *create_request(const char *file,
	const int lineno, unsigned int type, unsigned int flags,
	const void *payload, size_t psize)
{
	struct tst_rtnl_context *ctx;
	struct nlmsghdr header = {
		.nlmsg_type = type,
		.nlmsg_flags = NLM_F_REQUEST | NLM_F_ACK | flags,
	};

	ctx = tst_rtnl_create_context(file, lineno);

	if (!ctx)
		return NULL;

	if (!tst_rtnl_add_message(file, lineno, ctx, &header, payload, psize)) {
		tst_rtnl_destroy_context(file, lineno, ctx);
		return NULL;
	}

	return ctx;
}

int tst_netdev_index_by_name(const char *file, const int lineno,
	const char *ifname)
{
	struct ifreq ifr;
	int sock, ret;

	if (strlen(ifname) >= IFNAMSIZ) {
		tst_brk_(file, lineno, TBROK,
			"Network device name \"%s\" too long", ifname);
		return -1;
	}

	sock = safe_socket(file, lineno, NULL, AF_INET, SOCK_DGRAM, 0);

	if (sock < 0)
		return -1;

	strcpy(ifr.ifr_name, ifname);
	ret = SAFE_IOCTL_(file, lineno, sock, SIOCGIFINDEX, &ifr);
	safe_close(file, lineno, NULL, sock);

	return ret ? -1 : ifr.ifr_ifindex;
}

int tst_netdev_set_state(const char *file, const int lineno,
	const char *ifname, int up)
{
	struct ifreq ifr;
	int sock, ret;

	if (strlen(ifname) >= IFNAMSIZ) {
		tst_brk_(file, lineno, TBROK,
			"Network device name \"%s\" too long", ifname);
		return -1;
	}

	sock = safe_socket(file, lineno, NULL, AF_INET, SOCK_DGRAM, 0);

	if (sock < 0)
		return -1;

	strcpy(ifr.ifr_name, ifname);
	ret = SAFE_IOCTL_(file, lineno, sock, SIOCGIFFLAGS, &ifr);

	if (ret) {
		safe_close(file, lineno, NULL, sock);
		return ret;
	}

	if (up)
		ifr.ifr_flags |= IFF_UP;
	else
		ifr.ifr_flags &= ~IFF_UP;

	ret = SAFE_IOCTL_(file, lineno, sock, SIOCSIFFLAGS, &ifr);
	safe_close(file, lineno, NULL, sock);

	return ret;
}

int tst_create_veth_pair(const char *file, const int lineno,
	const char *ifname1, const char *ifname2)
{
	int ret;
	struct ifinfomsg info = { .ifi_family = AF_UNSPEC };
	struct tst_rtnl_context *ctx;
	struct tst_rtnl_attr_list peerinfo[] = {
		{IFLA_IFNAME, ifname2, strlen(ifname2) + 1, NULL},
		{0, NULL, -1, NULL}
	};
	struct tst_rtnl_attr_list peerdata[] = {
		{VETH_INFO_PEER, &info, sizeof(info), peerinfo},
		{0, NULL, -1, NULL}
	};
	struct tst_rtnl_attr_list attrs[] = {
		{IFLA_IFNAME, ifname1, strlen(ifname1) + 1, NULL},
		{IFLA_LINKINFO, NULL, 0, (const struct tst_rtnl_attr_list[]){
			{IFLA_INFO_KIND, "veth", 4, NULL},
			{IFLA_INFO_DATA, NULL, 0, peerdata},
			{0, NULL, -1, NULL}
		}},
		{0, NULL, -1, NULL}
	};

	if (strlen(ifname1) >= IFNAMSIZ) {
		tst_brk_(file, lineno, TBROK,
			"Network device name \"%s\" too long", ifname1);
		return 0;
	}

	if (strlen(ifname2) >= IFNAMSIZ) {
		tst_brk_(file, lineno, TBROK,
			"Network device name \"%s\" too long", ifname2);
		return 0;
	}

	ctx = create_request(file, lineno, RTM_NEWLINK,
		NLM_F_CREATE | NLM_F_EXCL, &info, sizeof(info));

	if (!ctx)
		return 0;

	if (tst_rtnl_add_attr_list(file, lineno, ctx, attrs) != 2) {
		tst_rtnl_destroy_context(file, lineno, ctx);
		return 0;
	}

	ret = tst_rtnl_send_validate(file, lineno, ctx);
	tst_rtnl_destroy_context(file, lineno, ctx);

	if (!ret) {
		tst_brk_(file, lineno, TBROK,
			"Failed to create veth interfaces %s+%s: %s", ifname1,
			ifname2, tst_strerrno(tst_rtnl_errno));
	}

	return ret;
}

int tst_netdev_add_device(const char *file, const int lineno,
	const char *ifname, const char *devtype)
{
	int ret;
	struct ifinfomsg info = { .ifi_family = AF_UNSPEC };
	struct tst_rtnl_context *ctx;
	struct tst_rtnl_attr_list attrs[] = {
		{IFLA_IFNAME, ifname, strlen(ifname) + 1, NULL},
		{IFLA_LINKINFO, NULL, 0, (const struct tst_rtnl_attr_list[]){
			{IFLA_INFO_KIND, devtype, strlen(devtype), NULL},
			{0, NULL, -1, NULL}
		}},
		{0, NULL, -1, NULL}
	};

	if (strlen(ifname) >= IFNAMSIZ) {
		tst_brk_(file, lineno, TBROK,
			"Network device name \"%s\" too long", ifname);
		return 0;
	}

	ctx = create_request(file, lineno, RTM_NEWLINK,
		NLM_F_CREATE | NLM_F_EXCL, &info, sizeof(info));

	if (!ctx)
		return 0;

	if (tst_rtnl_add_attr_list(file, lineno, ctx, attrs) != 2) {
		tst_rtnl_destroy_context(file, lineno, ctx);
		return 0;
	}

	ret = tst_rtnl_send_validate(file, lineno, ctx);
	tst_rtnl_destroy_context(file, lineno, ctx);

	if (!ret) {
		tst_brk_(file, lineno, TBROK,
			"Failed to create %s device %s: %s", devtype, ifname,
			tst_strerrno(tst_rtnl_errno));
	}

	return ret;
}

int tst_netdev_remove_device(const char *file, const int lineno,
	const char *ifname)
{
	struct ifinfomsg info = { .ifi_family = AF_UNSPEC };
	struct tst_rtnl_context *ctx;
	int ret;

	if (strlen(ifname) >= IFNAMSIZ) {
		tst_brk_(file, lineno, TBROK,
			"Network device name \"%s\" too long", ifname);
		return 0;
	}

	ctx = create_request(file, lineno, RTM_DELLINK, 0, &info, sizeof(info));

	if (!ctx)
		return 0;

	if (!tst_rtnl_add_attr_string(file, lineno, ctx, IFLA_IFNAME, ifname)) {
		tst_rtnl_destroy_context(file, lineno, ctx);
		return 0;
	}

	ret = tst_rtnl_send_validate(file, lineno, ctx);
	tst_rtnl_destroy_context(file, lineno, ctx);

	if (!ret) {
		tst_brk_(file, lineno, TBROK,
			"Failed to remove netdevice %s: %s", ifname,
			tst_strerrno(tst_rtnl_errno));
	}

	return ret;
}

static int modify_address(const char *file, const int lineno,
	unsigned int action, unsigned int nl_flags, const char *ifname,
	unsigned int family, const void *address, unsigned int prefix,
	size_t addrlen, uint32_t addr_flags)
{
	struct tst_rtnl_context *ctx;
	int index, ret;
	struct ifaddrmsg info = {
		.ifa_family = family,
		.ifa_prefixlen = prefix
	};

	index = tst_netdev_index_by_name(file, lineno, ifname);

	if (index < 0) {
		tst_brk_(file, lineno, TBROK, "Interface %s not found", ifname);
		return 0;
	}

	info.ifa_index = index;
	ctx = create_request(file, lineno, action, nl_flags, &info,
		sizeof(info));

	if (!ctx)
		return 0;

	if (!tst_rtnl_add_attr(file, lineno, ctx, IFA_FLAGS, &addr_flags,
		sizeof(uint32_t))) {
		tst_rtnl_destroy_context(file, lineno, ctx);
		return 0;
	}

	if (!tst_rtnl_add_attr(file, lineno, ctx, IFA_LOCAL, address,
		addrlen)) {
		tst_rtnl_destroy_context(file, lineno, ctx);
		return 0;
	}

	ret = tst_rtnl_send_validate(file, lineno, ctx);
	tst_rtnl_destroy_context(file, lineno, ctx);

	if (!ret) {
		tst_brk_(file, lineno, TBROK,
			"Failed to modify %s network address: %s", ifname,
			tst_strerrno(tst_rtnl_errno));
	}

	return ret;
}

int tst_netdev_add_address(const char *file, const int lineno,
	const char *ifname, unsigned int family, const void *address,
	unsigned int prefix, size_t addrlen, unsigned int flags)
{
	return modify_address(file, lineno, RTM_NEWADDR,
		NLM_F_CREATE | NLM_F_EXCL, ifname, family, address, prefix,
		addrlen, flags);
}

int tst_netdev_add_address_inet(const char *file, const int lineno,
	const char *ifname, in_addr_t address, unsigned int prefix,
	unsigned int flags)
{
	return tst_netdev_add_address(file, lineno, ifname, AF_INET,
		&address, prefix, sizeof(address), flags);
}

int tst_netdev_remove_address(const char *file, const int lineno,
	const char *ifname, unsigned int family, const void *address,
	size_t addrlen)
{
	return modify_address(file, lineno, RTM_DELADDR, 0, ifname, family,
		address, 0, addrlen, 0);
}

int tst_netdev_remove_address_inet(const char *file, const int lineno,
	const char *ifname, in_addr_t address)
{
	return tst_netdev_remove_address(file, lineno, ifname, AF_INET,
		&address, sizeof(address));
}

static int change_ns(const char *file, const int lineno, const char *ifname,
	unsigned short attr, uint32_t value)
{
	struct ifinfomsg info = { .ifi_family = AF_UNSPEC };
	struct tst_rtnl_context *ctx;
	int ret;

	if (strlen(ifname) >= IFNAMSIZ) {
		tst_brk_(file, lineno, TBROK,
			"Network device name \"%s\" too long", ifname);
		return 0;
	}

	ctx = create_request(file, lineno, RTM_NEWLINK, 0, &info, sizeof(info));

	if (!ctx)
		return 0;

	if (!tst_rtnl_add_attr_string(file, lineno, ctx, IFLA_IFNAME, ifname)) {
		tst_rtnl_destroy_context(file, lineno, ctx);
		return 0;
	}

	if (!tst_rtnl_add_attr(file, lineno, ctx, attr, &value,
		sizeof(uint32_t))) {
		tst_rtnl_destroy_context(file, lineno, ctx);
		return 0;
	}

	ret = tst_rtnl_send_validate(file, lineno, ctx);
	tst_rtnl_destroy_context(file, lineno, ctx);

	if (!ret) {
		tst_brk_(file, lineno, TBROK,
			"Failed to move %s to another namespace: %s", ifname,
			tst_strerrno(tst_rtnl_errno));
	}

	return ret;
}

int tst_netdev_change_ns_fd(const char *file, const int lineno,
	const char *ifname, int nsfd)
{
	return change_ns(file, lineno, ifname, IFLA_NET_NS_FD, nsfd);
}

int tst_netdev_change_ns_pid(const char *file, const int lineno,
	const char *ifname, pid_t nspid)
{
	return change_ns(file, lineno, ifname, IFLA_NET_NS_PID, nspid);
}

static int modify_route(const char *file, const int lineno, unsigned int action,
	unsigned int flags, const char *ifname, unsigned int family,
	const void *srcaddr, unsigned int srcprefix, size_t srclen,
	const void *dstaddr, unsigned int dstprefix, size_t dstlen,
	const void *gateway, size_t gatewaylen)
{
	struct tst_rtnl_context *ctx;
	int ret;
	int32_t index;
	struct rtmsg info = {
		.rtm_family = family,
		.rtm_dst_len = dstprefix,
		.rtm_src_len = srcprefix,
		.rtm_table = RT_TABLE_MAIN,
		.rtm_protocol = RTPROT_STATIC,
		.rtm_type = RTN_UNICAST
	};

	if (!ifname && !gateway) {
		tst_brk_(file, lineno, TBROK,
			"Interface name or gateway address required");
		return 0;
	}

	if (ifname && strlen(ifname) >= IFNAMSIZ) {
		tst_brk_(file, lineno, TBROK,
			"Network device name \"%s\" too long", ifname);
		return 0;
	}

	if (ifname) {
		index = tst_netdev_index_by_name(file, lineno, ifname);

		if (index < 0)
			return 0;
	}

	if (action == RTM_DELROUTE)
		info.rtm_scope = RT_SCOPE_NOWHERE;
	else
		info.rtm_scope = RT_SCOPE_UNIVERSE;

	ctx = create_request(file, lineno, action, flags, &info, sizeof(info));

	if (!ctx)
		return 0;

	if (srcaddr && !tst_rtnl_add_attr(file, lineno, ctx, RTA_SRC, srcaddr,
		srclen)) {
		tst_rtnl_destroy_context(file, lineno, ctx);
		return 0;
	}

	if (dstaddr && !tst_rtnl_add_attr(file, lineno, ctx, RTA_DST, dstaddr,
		dstlen)) {
		tst_rtnl_destroy_context(file, lineno, ctx);
		return 0;
	}

	if (gateway && !tst_rtnl_add_attr(file, lineno, ctx, RTA_GATEWAY,
		gateway, gatewaylen)) {
		tst_rtnl_destroy_context(file, lineno, ctx);
		return 0;
	}

	if (ifname && !tst_rtnl_add_attr(file, lineno, ctx, RTA_OIF, &index,
		sizeof(index))) {
		tst_rtnl_destroy_context(file, lineno, ctx);
		return 0;
	}

	ret = tst_rtnl_send_validate(file, lineno, ctx);
	tst_rtnl_destroy_context(file, lineno, ctx);

	if (!ret) {
		tst_brk_(file, lineno, TBROK,
			"Failed to modify network route: %s",
			tst_strerrno(tst_rtnl_errno));
	}

	return ret;
}

static int modify_route_inet(const char *file, const int lineno,
	unsigned int action, unsigned int flags, const char *ifname,
	in_addr_t srcaddr, unsigned int srcprefix, in_addr_t dstaddr,
	unsigned int dstprefix, in_addr_t gateway)
{
	void *src = NULL, *dst = NULL, *gw = NULL;
	size_t srclen = 0, dstlen = 0, gwlen = 0;

	if (srcprefix) {
		src = &srcaddr;
		srclen = sizeof(srcaddr);
	}

	if (dstprefix) {
		dst = &dstaddr;
		dstlen = sizeof(dstaddr);
	}

	if (gateway) {
		gw = &gateway;
		gwlen = sizeof(gateway);
	}

	return modify_route(file, lineno, action, flags, ifname, AF_INET, src,
		srcprefix, srclen, dst, dstprefix, dstlen, gw, gwlen);
}

int tst_netdev_add_route(const char *file, const int lineno,
	const char *ifname, unsigned int family, const void *srcaddr,
	unsigned int srcprefix, size_t srclen, const void *dstaddr,
	unsigned int dstprefix, size_t dstlen, const void *gateway,
	size_t gatewaylen)
{
	return modify_route(file, lineno, RTM_NEWROUTE,
		NLM_F_CREATE | NLM_F_EXCL, ifname, family, srcaddr, srcprefix,
		srclen, dstaddr, dstprefix, dstlen, gateway, gatewaylen);
}

int tst_netdev_add_route_inet(const char *file, const int lineno,
	const char *ifname, in_addr_t srcaddr, unsigned int srcprefix,
	in_addr_t dstaddr, unsigned int dstprefix, in_addr_t gateway)
{
	return modify_route_inet(file, lineno, RTM_NEWROUTE,
		NLM_F_CREATE | NLM_F_EXCL, ifname, srcaddr, srcprefix, dstaddr,
		dstprefix, gateway);
}

int tst_netdev_remove_route(const char *file, const int lineno,
	const char *ifname, unsigned int family, const void *srcaddr,
	unsigned int srcprefix, size_t srclen, const void *dstaddr,
	unsigned int dstprefix, size_t dstlen, const void *gateway,
	size_t gatewaylen)
{
	return modify_route(file, lineno, RTM_DELROUTE, 0, ifname, family,
		srcaddr, srcprefix, srclen, dstaddr, dstprefix, dstlen,
		gateway, gatewaylen);
}

int tst_netdev_remove_route_inet(const char *file, const int lineno,
	const char *ifname, in_addr_t srcaddr, unsigned int srcprefix,
	in_addr_t dstaddr, unsigned int dstprefix, in_addr_t gateway)
{
	return modify_route_inet(file, lineno, RTM_DELROUTE, 0, ifname,
		srcaddr, srcprefix, dstaddr, dstprefix, gateway);
}
