// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2017-2019 Petr Vorel <pvorel@suse.cz>
 */

#ifndef TST_NET_H_
#define TST_NET_H_

#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <sys/types.h>

void tst_get_in_addr(const char *ip_str, struct in_addr *ip);
void tst_get_in6_addr(const char *ip_str, struct in6_addr *ip6);

/*
 * Find valid connection address for a given bound socket
 */
socklen_t tst_get_connect_address(int sock, struct sockaddr_storage *addr);

/*
 * Initialize AF_INET/AF_INET6 socket address structure with address and port
 */
void tst_init_sockaddr_inet(struct sockaddr_in *sa, const char *ip_str, uint16_t port);
void tst_init_sockaddr_inet_bin(struct sockaddr_in *sa, uint32_t ip_val, uint16_t port);
void tst_init_sockaddr_inet6(struct sockaddr_in6 *sa, const char *ip_str, uint16_t port);
void tst_init_sockaddr_inet6_bin(struct sockaddr_in6 *sa, const struct in6_addr *ip_val, uint16_t port);

void safe_getaddrinfo(const char *file, const int lineno, const char *src_addr,
					  const char *port, const struct addrinfo *hints,
					  struct addrinfo **addr_info);

/*
 * Create new network namespace for netdevice/socket tests. A test which calls
 * tst_setup_netns() must declare the following entries in its struct tst_test:
 *
 * .needs_kconfigs = (const char *[]) {
 *	"CONFIG_USER_NS=y",
 *	"CONFIG_NET_NS=y",
 *	NULL
 * },
 * .save_restore = (const struct tst_path_val[]) {
 *	{"/proc/sys/user/max_user_namespaces", "1024", TST_SR_SKIP},
 *	{}
 * },
 */
void tst_setup_netns(void);

#endif /* TST_NET_H_ */
