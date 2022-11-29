// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2017-2019 Petr Vorel <pvorel@suse.cz>
 * Copyright (c) 2019 Martin Doucha <mdoucha@suse.cz>
 */

#include <errno.h>
#include <netdb.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#define TST_NO_DEFAULT_MAIN
#include "tst_test.h"
#include "tst_net.h"
#include "tst_private.h"
#include "lapi/sched.h"

void tst_print_svar(const char *name, const char *val)
{
	if (name && val)
		printf("export %s=\"%s\"\n", name, val);
}

void tst_print_svar_change(const char *name, const char *val)
{
	if (name && val)
		printf("export %s=\"${%s:-%s}\"\n", name, name, val);
}

/*
 * Function bit_count is from ipcalc project, ipcalc.c.
 */
static int tst_bit_count(uint32_t i)
{
	int c = 0;
	unsigned int seen_one = 0;

	while (i > 0) {
		if (i & 1) {
			seen_one = 1;
			c++;
		} else {
			if (seen_one)
				return -1;
		}
		i >>= 1;
	}

	return c;
}

/*
 * Function mask2prefix is from ipcalc project, ipcalc.c.
 */
static int tst_mask2prefix(struct in_addr mask)
{
	return tst_bit_count(ntohl(mask.s_addr));
}

/*
 * Function ipv4_mask_to_int is from ipcalc project, ipcalc.c.
 */
static int tst_ipv4_mask_to_int(const char *prefix)
{
	int ret;
	struct in_addr in;

	ret = inet_pton(AF_INET, prefix, &in);
	if (ret == 0)
		return -1;

	return tst_mask2prefix(in);
}

/*
 * Function safe_atoi is from ipcalc project, ipcalc.c.
 */
static int tst_safe_atoi(const char *s, int *ret_i)
{
	char *x = NULL;
	long l;

	errno = 0;
	l = strtol(s, &x, 0);

	if (!x || x == s || *x || errno)
		return errno > 0 ? -errno : -EINVAL;

	if ((long)(int)l != l)
		return -ERANGE;

	*ret_i = (int)l;

	return 0;
}

/*
 * Function get_prefix use code from ipcalc project, str_to_prefix/ipcalc.c.
 */
int tst_get_prefix(const char *ip_str, int is_ipv6)
{
	char *prefix_str = NULL;
	int prefix = -1, r;

	prefix_str = strchr(ip_str, '/');
	if (!prefix_str)
		return -1;

	*(prefix_str++) = '\0';

	if (!is_ipv6 && strchr(prefix_str, '.'))
		prefix = tst_ipv4_mask_to_int(prefix_str);
	else {
		r = tst_safe_atoi(prefix_str, &prefix);
		if (r != 0)
			tst_brk_comment("conversion error: '%s' is not integer",
					prefix_str);
	}

	if (prefix < 0 || ((is_ipv6 && prefix > MAX_IPV6_PREFIX) ||
		(!is_ipv6 && prefix > MAX_IPV4_PREFIX)))
		tst_brk_comment("bad %s prefix: %s", is_ipv6 ?  "IPv6" : "IPv4",
				prefix_str);

	return prefix;
}

void tst_get_in_addr(const char *ip_str, struct in_addr *ip)
{
	if (inet_pton(AF_INET, ip_str, ip) <= 0)
		tst_brk_comment("bad IPv4 address: '%s'", ip_str);
}

void tst_get_in6_addr(const char *ip_str, struct in6_addr *ip6)
{
	if (inet_pton(AF_INET6, ip_str, ip6) <= 0)
		tst_brk_comment("bad IPv6 address: '%s'", ip_str);
}

socklen_t tst_get_connect_address(int sock, struct sockaddr_storage *addr)
{
	struct sockaddr_in *inet_ptr;
	struct sockaddr_in6 *inet6_ptr;
	size_t tmp_size;
	socklen_t ret = sizeof(*addr);

	SAFE_GETSOCKNAME(sock, (struct sockaddr*)addr, &ret);

	/* Sanitize wildcard addresses */
	switch (addr->ss_family) {
	case AF_INET:
		inet_ptr = (struct sockaddr_in*)addr;

		switch (ntohl(inet_ptr->sin_addr.s_addr)) {
		case INADDR_ANY:
		case INADDR_BROADCAST:
			inet_ptr->sin_addr.s_addr = htonl(INADDR_LOOPBACK);
			break;
		}

		break;

	case AF_INET6:
		inet6_ptr = (struct sockaddr_in6*)addr;
		tmp_size = sizeof(struct in6_addr);

		if (!memcmp(&inet6_ptr->sin6_addr, &in6addr_any, tmp_size)) {
			memcpy(&inet6_ptr->sin6_addr, &in6addr_loopback,
				tmp_size);
		}

		break;
	}

	return ret;
}

void tst_init_sockaddr_inet(struct sockaddr_in *sa, const char *ip_str, uint16_t port)
{
	memset(sa, 0, sizeof(struct sockaddr_in));
	sa->sin_family = AF_INET;
	sa->sin_port = htons(port);
	tst_get_in_addr(ip_str, &sa->sin_addr);
}

void tst_init_sockaddr_inet_bin(struct sockaddr_in *sa, uint32_t ip_val, uint16_t port)
{
	memset(sa, 0, sizeof(struct sockaddr_in));
	sa->sin_family = AF_INET;
	sa->sin_port = htons(port);
	sa->sin_addr.s_addr = htonl(ip_val);
}

void tst_init_sockaddr_inet6(struct sockaddr_in6 *sa, const char *ip_str, uint16_t port)
{
	memset(sa, 0, sizeof(struct sockaddr_in6));
	sa->sin6_family = AF_INET6;
	sa->sin6_port = htons(port);
	tst_get_in6_addr(ip_str, &sa->sin6_addr);
}

void tst_init_sockaddr_inet6_bin(struct sockaddr_in6 *sa, const struct in6_addr *ip_val, uint16_t port)
{
	memset(sa, 0, sizeof(struct sockaddr_in6));
	sa->sin6_family = AF_INET6;
	sa->sin6_port = htons(port);
	memcpy(&sa->sin6_addr, ip_val, sizeof(struct in6_addr));
}

void safe_getaddrinfo(const char *file, const int lineno, const char *src_addr,
					  const char *port, const struct addrinfo *hints,
					  struct addrinfo **addr_info)
{
	int err = getaddrinfo(src_addr, port, hints, addr_info);

	if (err) {
		tst_brk_(file, lineno, TBROK, "getaddrinfo failed, %s",
			gai_strerror(err));
	}

	if (!*addr_info)
		tst_brk_(file, lineno, TBROK, "failed to get the address");
}

void tst_setup_netns(void)
{
	int real_uid = getuid();
	int real_gid = getgid();
	int nscount = 1;

	if (!access("/proc/sys/user/max_user_namespaces", F_OK)) {
		SAFE_FILE_SCANF("/proc/sys/user/max_user_namespaces", "%d",
			&nscount);
	}

	if (!nscount)
		tst_brk(TCONF, "User namespaces are disabled");

	SAFE_UNSHARE(CLONE_NEWUSER);
	SAFE_UNSHARE(CLONE_NEWNET);
	SAFE_FILE_PRINTF("/proc/self/setgroups", "deny");
	SAFE_FILE_PRINTF("/proc/self/uid_map", "0 %d 1", real_uid);
	SAFE_FILE_PRINTF("/proc/self/gid_map", "0 %d 1", real_gid);
}
