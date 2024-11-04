// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2024 SUSE LLC Andrea Cervesato <andrea.cervesato@suse.com>
 */

/*\
 * [Description]
 *
 * Verify the landlock support for bind()/connect() syscalls in IPV4 and IPV6
 * protocols. In particular, check that bind() is assigning the address only on
 * the TCP port enforced by LANDLOCK_ACCESS_NET_BIND_TCP and check that
 * connect() is connecting only to a specific TCP port enforced by
 * LANDLOCK_ACCESS_NET_CONNECT_TCP.
 *
 * [Algorithm]
 *
 * Repeat the following procedure for IPV4 and IPV6:
 *
 * - create a socket on PORT1, bind() it and check if it passes
 * - enforce the current sandbox with LANDLOCK_ACCESS_NET_BIND_TCP on PORT1
 * - create a socket on PORT1, bind() it and check if it passes
 * - create a socket on PORT2, bind() it and check if it fails
 *
 * - create a server listening on PORT1
 * - create a socket on PORT1, connect() to it and check if it passes
 * - enforce the current sandbox with LANDLOCK_ACCESS_NET_CONNECT_TCP on PORT1
 * - create a socket on PORT1, connect() to it and check if it passes
 * - create a socket on PORT2, connect() to it and check if it fails
 */

#include "landlock_common.h"

static int variants[] = {
	AF_INET,
	AF_INET6,
};

static struct tst_landlock_ruleset_attr_abi4 *ruleset_attr;
static struct landlock_net_port_attr *net_port_attr;
static in_port_t *server_port;
static int addr_port;

static void create_server(const int addr_family)
{
	struct socket_data socket;
	struct sockaddr *addr = NULL;

	create_socket(&socket, addr_family, 0);
	getsocket_addr(&socket, addr_family, &addr);

	SAFE_BIND(socket.fd, addr, socket.address_size);
	SAFE_LISTEN(socket.fd, 1);

	*server_port = getsocket_port(&socket, addr_family);

	tst_res(TDEBUG, "Server listening on port %u", *server_port);

	TST_CHECKPOINT_WAKE_AND_WAIT(0);

	SAFE_CLOSE(socket.fd);
}

static void test_bind(const int addr_family, const in_port_t port,
	const int exp_err)
{
	struct socket_data socket;
	struct sockaddr *addr = NULL;

	create_socket(&socket, addr_family, port);
	getsocket_addr(&socket, addr_family, &addr);

	if (exp_err) {
		TST_EXP_FAIL(
			bind(socket.fd, addr, socket.address_size),
			exp_err, "bind() access on port %u", port);
	} else {
		TST_EXP_PASS(
			bind(socket.fd, addr, socket.address_size),
			"bind() access on port %u", port);
	}

	SAFE_CLOSE(socket.fd);
}

static void test_connect(const int addr_family, const in_port_t port,
	const int exp_err)
{
	struct socket_data socket;
	struct sockaddr *addr = NULL;

	create_socket(&socket, addr_family, port);
	getsocket_addr(&socket, addr_family, &addr);

	if (exp_err) {
		TST_EXP_FAIL(
			connect(socket.fd, addr, socket.address_size),
			exp_err, "connect() on port %u", port);
	} else {
		TST_EXP_PASS(
			connect(socket.fd, addr, socket.address_size),
			"connect() on port %u", port);
	}

	SAFE_CLOSE(socket.fd);
}

static void run(void)
{
	int addr_family = variants[tst_variant];

	tst_res(TINFO, "Using %s protocol",
		addr_family == AF_INET ? "IPV4" : "IPV6");

	if (!SAFE_FORK()) {
		create_server(addr_family);
		exit(0);
	}

	TST_CHECKPOINT_WAIT(0);

	/* verify bind() syscall accessibility */
	if (!SAFE_FORK()) {
		ruleset_attr->handled_access_net =
			LANDLOCK_ACCESS_NET_BIND_TCP;

		test_bind(addr_family, addr_port, 0);

		tst_res(TINFO, "Enable bind() access only for port %u",
			addr_port);

		apply_landlock_net_layer(
			ruleset_attr,
			sizeof(struct tst_landlock_ruleset_attr_abi4),
			net_port_attr,
			addr_port,
			LANDLOCK_ACCESS_NET_BIND_TCP);

		test_bind(addr_family, addr_port, 0);
		test_bind(addr_family, addr_port + 0x80, EACCES);

		exit(0);
	}

	/* verify connect() syscall accessibility */
	if (!SAFE_FORK()) {
		ruleset_attr->handled_access_net =
			LANDLOCK_ACCESS_NET_CONNECT_TCP;

		test_connect(addr_family, *server_port, 0);

		tst_res(TINFO, "Enable connect() access only on port %u",
			*server_port);

		apply_landlock_net_layer(
			ruleset_attr,
			sizeof(struct tst_landlock_ruleset_attr_abi4),
			net_port_attr,
			*server_port,
			LANDLOCK_ACCESS_NET_CONNECT_TCP);

		test_connect(addr_family, *server_port, 0);
		test_connect(addr_family, *server_port + 0x80, EACCES);

		TST_CHECKPOINT_WAKE(0);

		exit(0);
	}
}

static void setup(void)
{
	if (verify_landlock_is_enabled() < 4)
		tst_brk(TCONF, "Landlock network is not supported");

	addr_port = TST_GET_UNUSED_PORT(AF_INET, SOCK_STREAM);

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
