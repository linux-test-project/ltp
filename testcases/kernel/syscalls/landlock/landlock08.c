// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2024 SUSE LLC Andrea Cervesato <andrea.cervesato@suse.com>
 */

/*\
 * Verify the landlock support for :manpage:`bind(2)` and :manpage:`connect(2)`
 * syscalls in IPv4 and IPv6 protocols, using both TCP and UDP. In particular,
 * check that :manpage:`bind(2)` is assigning the address only on the port
 * enforced by LANDLOCK_ACCESS_NET_BIND_TCP / LANDLOCK_ACCESS_NET_BIND_UDP
 * and check that :manpage:`connect(2)` is connecting only to a specific port
 * enforced by LANDLOCK_ACCESS_NET_CONNECT_TCP / LANDLOCK_ACCESS_NET_CONNECT_SEND_UDP.
 *
 * TCP rules are available since Landlock ABI v4, while UDP rules are available
 * since Landlock ABI v10.
 *
 * [Algorithm]
 *
 * Repeat the following procedure for {TCP, UDP} x {IPv4, IPv6}:
 *
 * - create a socket on PORT1, :manpage:`bind(2)` it and check if it passes
 * - enforce the current sandbox with the BIND access right on PORT1
 * - create a socket on PORT1, :manpage:`bind(2)` it and check if it passes
 * - create a socket on PORT2, :manpage:`bind(2)` it and check if it fails
 *
 * - create a server on PORT1 (listening for TCP, bound for UDP)
 * - create a socket on PORT1, :manpage:`connect(2)` to it and check if it passes
 * - enforce the current sandbox with the CONNECT access right on PORT1
 * - create a socket on PORT1, :manpage:`connect(2)` to it and check if it passes
 * - create a socket on PORT2, :manpage:`connect(2)` to it and check if it fails
 */

#include "landlock_common.h"

static struct tcase {
	int family;
	int type;
	uint64_t bind_access;
	uint64_t connect_access;
	int min_abi;
	const char *desc;
} variants[] = {
	{
		AF_INET, SOCK_STREAM,
		LANDLOCK_ACCESS_NET_BIND_TCP,
		LANDLOCK_ACCESS_NET_CONNECT_TCP,
		4, "TCP/IPv4"
	},
	{
		AF_INET6, SOCK_STREAM,
		LANDLOCK_ACCESS_NET_BIND_TCP,
		LANDLOCK_ACCESS_NET_CONNECT_TCP,
		4, "TCP/IPv6"
	},
	{
		AF_INET, SOCK_DGRAM,
		LANDLOCK_ACCESS_NET_BIND_UDP,
		LANDLOCK_ACCESS_NET_CONNECT_SEND_UDP,
		10, "UDP/IPv4"
	},
	{
		AF_INET6, SOCK_DGRAM,
		LANDLOCK_ACCESS_NET_BIND_UDP,
		LANDLOCK_ACCESS_NET_CONNECT_SEND_UDP,
		10, "UDP/IPv6"
	},
};

static struct tst_landlock_ruleset_attr_abi4 *ruleset_attr;
static struct landlock_net_port_attr *net_port_attr;
static in_port_t *server_port;
static int addr_port;
static int landlock_abi;

static void create_server(const struct tcase *tc)
{
	struct socket_data socket;
	struct sockaddr *addr = NULL;

	create_socket(&socket, tc->family, 0, tc->type);
	getsocket_addr(&socket, tc->family, &addr);

	SAFE_BIND(socket.fd, addr, socket.address_size);

	if (tc->type == SOCK_STREAM)
		SAFE_LISTEN(socket.fd, 1);

	*server_port = getsocket_port(&socket, tc->family);

	tst_res(TDEBUG, "Server bound on port %u", *server_port);

	TST_CHECKPOINT_WAKE_AND_WAIT(0);

	SAFE_CLOSE(socket.fd);
}

static void test_bind(const struct tcase *tc, const in_port_t port, const int exp_err)
{
	struct socket_data socket;
	struct sockaddr *addr = NULL;

	create_socket(&socket, tc->family, port, tc->type);
	getsocket_addr(&socket, tc->family, &addr);

	TST_EXP_PASS_OR_FAIL(bind(socket.fd, addr, socket.address_size),
		exp_err, "bind() access on port %u", port);

	SAFE_CLOSE(socket.fd);
}

static void test_connect(const struct tcase *tc, const in_port_t port, const int exp_err)
{
	struct socket_data socket;
	struct sockaddr *addr = NULL;

	create_socket(&socket, tc->family, port, tc->type);
	getsocket_addr(&socket, tc->family, &addr);

	TST_EXP_PASS_OR_FAIL(connect(socket.fd, addr, socket.address_size),
		exp_err, "connect() on port %u", port);

	SAFE_CLOSE(socket.fd);
}

static int check_family_support(const struct tcase *tc)
{
	int fd;

	fd = socket(tc->family, tc->type, 0);
	if (fd == -1 && errno == EAFNOSUPPORT) {
		tst_res(TCONF, "%s address family not supported in kernel",
			tc->family == AF_INET ? "IPv4" : "IPv6");
		return 0;
	}
	if (fd != -1)
		close(fd);
	return 1;
}

static void run(void)
{
	struct tcase *tc = &variants[tst_variant];

	tst_res(TINFO, "Using %s protocol", tc->desc);

	addr_port = TST_GET_UNUSED_PORT(tc->family, tc->type);

	if (landlock_abi < tc->min_abi) {
		tst_res(TCONF, "%s rules require Landlock ABI v%d",
			tc->desc, tc->min_abi);
		return;
	}

	if (!check_family_support(tc))
		return;

	if (!SAFE_FORK()) {
		create_server(tc);
		exit(0);
	}

	TST_CHECKPOINT_WAIT(0);

	/* verify bind() syscall accessibility */
	if (!SAFE_FORK()) {
		ruleset_attr->handled_access_net = tc->bind_access;

		test_bind(tc, addr_port, 0);

		tst_res(TINFO, "Enable bind() access only for port %u",
			addr_port);

		apply_landlock_net_layer(ruleset_attr,
					 sizeof(struct tst_landlock_ruleset_attr_abi4),
					 net_port_attr,
					 addr_port,
					 tc->bind_access);

		test_bind(tc, addr_port, 0);
		test_bind(tc, addr_port + 0x80, EACCES);

		exit(0);
	}

	/* verify connect() syscall accessibility */
	if (!SAFE_FORK()) {
		ruleset_attr->handled_access_net = tc->connect_access;

		test_connect(tc, *server_port, 0);

		tst_res(TINFO, "Enable connect() access only on port %u",
			*server_port);

		apply_landlock_net_layer(ruleset_attr,
					 sizeof(struct tst_landlock_ruleset_attr_abi4),
					 net_port_attr,
					 *server_port,
					 tc->connect_access);

		test_connect(tc, *server_port, 0);
		test_connect(tc, *server_port + 0x80, EACCES);

		TST_CHECKPOINT_WAKE(0);

		exit(0);
	}
}

static void setup(void)
{
	landlock_abi = verify_landlock_is_enabled();
	if (landlock_abi < 4)
		tst_brk(TCONF, "Landlock network is not supported");

	server_port = SAFE_MMAP(NULL, sizeof(in_port_t), PROT_READ | PROT_WRITE,
				MAP_SHARED | MAP_ANONYMOUS, -1, 0);
}

static void cleanup(void)
{
	if (server_port)
		SAFE_MUNMAP(server_port, sizeof(in_port_t));
}

static struct tst_test test = {
	.test_all = run,
	.setup = setup,
	.cleanup = cleanup,
	.needs_root = 1,
	.needs_checkpoints = 1,
	.forks_child = 1,
	.test_variants = ARRAY_SIZE(variants),
	.bufs = (struct tst_buffers[]) {
		{&ruleset_attr, .size = sizeof(struct tst_landlock_ruleset_attr_abi4)},
		{&net_port_attr, .size = sizeof(struct landlock_net_port_attr)},
		{},
	},
	.caps = (struct tst_cap []) {
		TST_CAP(TST_CAP_REQ, CAP_SYS_ADMIN),
		TST_CAP(TST_CAP_REQ, CAP_NET_BIND_SERVICE),
		{}
	},
	.needs_kconfigs = (const char *[]) {
		"CONFIG_INET=y",
		NULL
	},
};
