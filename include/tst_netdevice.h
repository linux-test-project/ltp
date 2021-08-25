/* SPDX-License-Identifier: GPL-2.0-or-later
 * Copyright (c) 2021 Linux Test Project
 */

#ifndef TST_NETDEVICE_H
#define TST_NETDEVICE_H

/* Find device index for given network interface name. */
int tst_netdev_index_by_name(const char *file, const int lineno,
	const char *ifname);
#define NETDEV_INDEX_BY_NAME(ifname) \
	tst_netdev_index_by_name(__FILE__, __LINE__, (ifname))

/* Activate or deactivate network interface */
int tst_netdev_set_state(const char *file, const int lineno,
	const char *ifname, int up);
#define NETDEV_SET_STATE(ifname, up) \
	tst_netdev_set_state(__FILE__, __LINE__, (ifname), (up))

/* Create a connected pair of virtual network devices */
int tst_create_veth_pair(const char *file, const int lineno,
	const char *ifname1, const char *ifname2);
#define CREATE_VETH_PAIR(ifname1, ifname2) \
	tst_create_veth_pair(__FILE__, __LINE__, (ifname1), (ifname2))

int tst_netdev_add_device(const char *file, const int lineno,
	const char *ifname, const char *devtype);
#define NETDEV_ADD_DEVICE(ifname, devtype) \
	tst_netdev_add_device(__FILE__, __LINE__, (ifname), (devtype))

int tst_remove_netdev(const char *file, const int lineno, const char *ifname);
#define REMOVE_NETDEV(ifname) tst_remove_netdev(__FILE__, __LINE__, (ifname))

int tst_netdev_add_address(const char *file, const int lineno,
	const char *ifname, unsigned int family, const void *address,
	unsigned int prefix, size_t addrlen, unsigned int flags);
#define NETDEV_ADD_ADDRESS(ifname, family, address, prefix, addrlen, flags) \
	tst_netdev_add_address(__FILE__, __LINE__, (ifname), (family), \
		(address), (prefix), (addrlen), (flags))

int tst_netdev_add_address_inet(const char *file, const int lineno,
	const char *ifname, in_addr_t address, unsigned int prefix,
	unsigned int flags);
#define NETDEV_ADD_ADDRESS_INET(ifname, address, prefix, flags) \
	tst_netdev_add_address_inet(__FILE__, __LINE__, (ifname), (address), \
		(prefix), (flags))

int tst_netdev_remove_address(const char *file, const int lineno,
	const char *ifname, unsigned int family, const void *address,
	size_t addrlen);
#define NETDEV_REMOVE_ADDRESS(ifname, family, address, addrlen) \
	tst_netdev_remove_address(__FILE__, __LINE__, (ifname), (family), \
		(address), (addrlen))

int tst_netdev_remove_address_inet(const char *file, const int lineno,
	const char *ifname, in_addr_t address);
#define NETDEV_REMOVE_ADDRESS_INET(ifname, address) \
	tst_netdev_remove_address_inet(__FILE__, __LINE__, (ifname), (address))

int tst_netdev_change_ns_fd(const char *file, const int lineno,
	const char *ifname, int nsfd);
#define NETDEV_CHANGE_NS_FD(ifname, nsfd) \
	tst_netdev_change_ns_fd(__FILE__, __LINE__, (ifname), (nsfd))

int tst_netdev_change_ns_pid(const char *file, const int lineno,
	const char *ifname, pid_t nspid);
#define NETDEV_CHANGE_NS_PID(ifname, nspid) \
	tst_netdev_change_ns_pid(__FILE__, __LINE__, (ifname), (nspid))

/*
 * Add new static entry to main routing table. If you specify gateway address,
 * the interface name is optional.
 */
int tst_netdev_add_route(const char *file, const int lineno,
	const char *ifname, unsigned int family, const void *srcaddr,
	unsigned int srcprefix, size_t srclen, const void *dstaddr,
	unsigned int dstprefix, size_t dstlen, const void *gateway,
	size_t gatewaylen);
#define NETDEV_ADD_ROUTE(ifname, family, srcaddr, srcprefix, srclen, dstaddr, \
	dstprefix, dstlen, gateway, gatewaylen) \
	tst_netdev_add_route(__FILE__, __LINE__, (ifname), (family), \
		(srcaddr), (srcprefix), (srclen), (dstaddr), (dstprefix), \
		(dstlen), (gateway), (gatewaylen))

/*
 * Simplified function for adding IPv4 static route. If you set srcprefix
 * or dstprefix to 0, the corresponding address will be ignored. Interface
 * name is optional if gateway address is non-zero.
 */
int tst_netdev_add_route_inet(const char *file, const int lineno,
	const char *ifname, in_addr_t srcaddr, unsigned int srcprefix,
	in_addr_t dstaddr, unsigned int dstprefix, in_addr_t gateway);
#define NETDEV_ADD_ROUTE_INET(ifname, srcaddr, srcprefix, dstaddr, dstprefix, \
	gateway) \
	tst_netdev_add_route_inet(__FILE__, __LINE__, (ifname), (srcaddr), \
		(srcprefix), (dstaddr), (dstprefix), (gateway))

/*
 * Remove static entry from main routing table.
 */
int tst_netdev_remove_route(const char *file, const int lineno,
	const char *ifname, unsigned int family, const void *srcaddr,
	unsigned int srcprefix, size_t srclen, const void *dstaddr,
	unsigned int dstprefix, size_t dstlen, const void *gateway,
	size_t gatewaylen);
#define NETDEV_REMOVE_ROUTE(ifname, family, srcaddr, srcprefix, srclen, \
	dstaddr, dstprefix, dstlen, gateway, gatewaylen) \
	tst_netdev_remove_route(__FILE__, __LINE__, (ifname), (family), \
		(srcaddr), (srcprefix), (srclen), (dstaddr), (dstprefix), \
		(dstlen), (gateway), (gatewaylen))

/*
 * Simplified function for removing IPv4 static route.
 */
int tst_netdev_remove_route_inet(const char *file, const int lineno,
	const char *ifname, in_addr_t srcaddr, unsigned int srcprefix,
	in_addr_t dstaddr, unsigned int dstprefix, in_addr_t gateway);
#define NETDEV_REMOVE_ROUTE_INET(ifname, srcaddr, srcprefix, dstaddr, \
	dstprefix, gateway) \
	tst_netdev_remove_route_inet(__FILE__, __LINE__, (ifname), (srcaddr), \
		(srcprefix), (dstaddr), (dstprefix), (gateway))

#endif /* TST_NETDEVICE_H */
