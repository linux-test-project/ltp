/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (C) 2024 SUSE LLC Andrea Cervesato <andrea.cervesato@suse.com>
 */

#ifndef LANDLOCK_COMMON_H__
#define LANDLOCK_COMMON_H__

#include "tst_test.h"
#include "lapi/prctl.h"
#include "lapi/fcntl.h"
#include "lapi/landlock.h"

#define IPV4_LOCALHOST "127.0.0.1"
#define IPV6_LOCALHOST "::1"

struct socket_data {
	struct sockaddr_in addr_ipv4;
	struct sockaddr_in6 addr_ipv6;
	size_t address_size;
	int fd;
};

static inline int verify_landlock_is_enabled(void)
{
	int abi;

	abi = tst_syscall(__NR_landlock_create_ruleset,
		NULL, 0, LANDLOCK_CREATE_RULESET_VERSION);

	if (abi < 0) {
		if (errno == EOPNOTSUPP) {
			tst_brk(TCONF, "Landlock is currently disabled. "
				"Please enable it either via CONFIG_LSM or "
				"'lsm' kernel parameter.");
		}

		tst_brk(TBROK | TERRNO, "landlock_create_ruleset error");
	}

	tst_res(TINFO, "Landlock ABI v%d", abi);

	return abi;
}

static inline void apply_landlock_fs_rule(
	struct landlock_path_beneath_attr *path_beneath_attr,
	const int ruleset_fd,
	const int access,
	const char *path)
{
	path_beneath_attr->allowed_access = access;
	path_beneath_attr->parent_fd = SAFE_OPEN(path, O_PATH | O_CLOEXEC);

	SAFE_LANDLOCK_ADD_RULE(
		ruleset_fd,
		LANDLOCK_RULE_PATH_BENEATH,
		path_beneath_attr,
		0);

	SAFE_CLOSE(path_beneath_attr->parent_fd);
}

static inline void apply_landlock_net_rule(
	struct landlock_net_port_attr *net_attr,
	const int ruleset_fd,
	const uint64_t port,
	const uint64_t access)
{
	net_attr->port = port;
	net_attr->allowed_access = access;

	SAFE_LANDLOCK_ADD_RULE(
		ruleset_fd,
		LANDLOCK_RULE_NET_PORT,
		net_attr,
		0);
}

static inline void enforce_ruleset(const int ruleset_fd)
{
	SAFE_PRCTL(PR_SET_NO_NEW_PRIVS, 1, 0, 0, 0);
	SAFE_LANDLOCK_RESTRICT_SELF(ruleset_fd, 0);
}

static inline void apply_landlock_fs_layer(
	void *ruleset_attr, size_t attr_size,
	struct landlock_path_beneath_attr *path_beneath_attr,
	const char *path,
	const int access)
{
	int ruleset_fd;

	ruleset_fd = SAFE_LANDLOCK_CREATE_RULESET(ruleset_attr, attr_size, 0);

	apply_landlock_fs_rule(path_beneath_attr, ruleset_fd, access, path);
	enforce_ruleset(ruleset_fd);

	SAFE_CLOSE(ruleset_fd);
}

static inline void apply_landlock_net_layer(
	void *ruleset_attr, size_t attr_size,
	struct landlock_net_port_attr *net_port_attr,
	const in_port_t port,
	const uint64_t access)
{
	int ruleset_fd;

	ruleset_fd = SAFE_LANDLOCK_CREATE_RULESET(ruleset_attr, attr_size, 0);

	apply_landlock_net_rule(net_port_attr, ruleset_fd, port, access);
	enforce_ruleset(ruleset_fd);

	SAFE_CLOSE(ruleset_fd);
}

static inline void apply_landlock_scoped_layer(
	void *ruleset_attr, size_t attr_size)
{
	int ruleset_fd;

	ruleset_fd = SAFE_LANDLOCK_CREATE_RULESET(ruleset_attr, attr_size, 0);
	enforce_ruleset(ruleset_fd);

	SAFE_CLOSE(ruleset_fd);
}

static inline in_port_t getsocket_port(struct socket_data *socket,
	const int addr_family)
{
	struct sockaddr_in addr_ipv4;
	struct sockaddr_in6 addr_ipv6;
	socklen_t len;
	in_port_t port = 0;

	switch (addr_family) {
	case AF_INET:
		len = sizeof(addr_ipv4);
		memset(&addr_ipv4, 0, len);

		SAFE_GETSOCKNAME(socket->fd, (struct sockaddr *)&addr_ipv4, &len);
		port = ntohs(addr_ipv4.sin_port);
		break;
	case AF_INET6:
		len = sizeof(addr_ipv6);
		memset(&addr_ipv6, 0, len);

		SAFE_GETSOCKNAME(socket->fd, (struct sockaddr *)&addr_ipv6, &len);
		port = ntohs(addr_ipv6.sin6_port);
		break;
	default:
		tst_brk(TBROK, "Unsupported protocol");
		break;
	};

	return port;
}

static inline void create_socket(struct socket_data *socket,
	const int addr_family, const in_port_t port)
{
	memset(socket, 0, sizeof(struct socket_data));

	switch (addr_family) {
	case AF_INET:
		if (!port) {
			tst_init_sockaddr_inet_bin(&socket->addr_ipv4,
				INADDR_ANY, 0);
		} else {
			tst_init_sockaddr_inet(&socket->addr_ipv4,
				IPV4_LOCALHOST, port);
		}

		socket->address_size = sizeof(struct sockaddr_in);
		break;
	case AF_INET6:
		if (!port) {
			tst_init_sockaddr_inet6_bin(&socket->addr_ipv6,
				&in6addr_any, 0);
		} else {
			tst_init_sockaddr_inet6(&socket->addr_ipv6,
				IPV6_LOCALHOST, port);
		}

		socket->address_size = sizeof(struct sockaddr_in6);
		break;
	default:
		tst_brk(TBROK, "Unsupported protocol");
		return;
	};

	socket->fd = SAFE_SOCKET(addr_family, SOCK_STREAM | SOCK_CLOEXEC, 0);
}

static inline void getsocket_addr(struct socket_data *socket,
	const int addr_family, struct sockaddr **addr)
{
	switch (addr_family) {
	case AF_INET:
		*addr = (struct sockaddr *)&socket->addr_ipv4;
		break;
	case AF_INET6:
		*addr = (struct sockaddr *)&socket->addr_ipv6;
		break;
	default:
		break;
	};
}
#endif /* LANDLOCK_COMMON_H__ */
