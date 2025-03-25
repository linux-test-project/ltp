// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2025 SUSE LLC Andrea Cervesato <andrea.cervesato@suse.com>
 */

/*\
 * Verify that landlock's LANDLOCK_SCOPE_ABSTRACT_UNIX_SOCKET rule reject any
 * connect() coming from a client on a different server domain, but accept any
 * connection.
 */

#include <stddef.h>
#include "tst_test.h"
#include "landlock_common.h"

#define SOCKET_NAME "test.sock"
#define ABSTRACT_SOCKET_NAME "\0"SOCKET_NAME
#define SOCKET_LENGTH (offsetof(struct sockaddr_un, sun_path) + strlen(SOCKET_NAME) + 1)

enum {
	DOMAIN_CLIENT = 0,
	DOMAIN_SERVER,
	DOMAIN_BOTH,
};

static struct tst_landlock_ruleset_attr_abi6 *ruleset_attr;

static void scoped_sandbox(const char *from)
{
	tst_res(TINFO, "Enforcing rule LANDLOCK_SCOPE_ABSTRACT_UNIX_SOCKET on %s", from);

	ruleset_attr->scoped = LANDLOCK_SCOPE_ABSTRACT_UNIX_SOCKET;
	apply_landlock_scoped_layer(ruleset_attr, sizeof(*ruleset_attr));
}

static void run_client(void)
{
	if (tst_variant == DOMAIN_CLIENT)
		scoped_sandbox("client");

	int sendsock;
	struct sockaddr_un addr = {
		.sun_family = AF_UNIX,
		.sun_path = ABSTRACT_SOCKET_NAME,
	};

	TST_CHECKPOINT_WAIT(0);

	tst_res(TINFO, "Connecting to UNIX socket");

	sendsock = SAFE_SOCKET(AF_UNIX, SOCK_STREAM, 0);

	if (tst_variant != DOMAIN_CLIENT)
		TST_EXP_PASS(connect(sendsock, (struct sockaddr *)&addr, SOCKET_LENGTH));
	else
		TST_EXP_FAIL(connect(sendsock, (struct sockaddr *)&addr, SOCKET_LENGTH), EPERM);

	SAFE_CLOSE(sendsock);

	TST_CHECKPOINT_WAKE(0);
}

static void run_server(void)
{
	if (tst_variant == DOMAIN_SERVER)
		scoped_sandbox("server");

	int recvsock;
	struct sockaddr_un addr = {
		.sun_family = AF_UNIX,
		.sun_path = ABSTRACT_SOCKET_NAME,
	};

	recvsock = SAFE_SOCKET(AF_UNIX, SOCK_STREAM, 0);

	SAFE_BIND(recvsock, (struct sockaddr *)&addr, SOCKET_LENGTH);
	SAFE_LISTEN(recvsock, 5);

	tst_res(TINFO, "Listening on UNIX socket");

	TST_CHECKPOINT_WAKE_AND_WAIT(0);

	SAFE_CLOSE(recvsock);
}

static void run(void)
{
	/* isolate test inside a process so we won't stack too many
	 * layers (-E2BIG) when there are multiple test's iterations
	 */
	if (SAFE_FORK())
		return;

	if (tst_variant == DOMAIN_BOTH)
		scoped_sandbox("server and client");

	if (!SAFE_FORK()) {
		run_client();
		exit(0);
	}

	run_server();

	tst_reap_children();
}

static void setup(void)
{
	int abi;

	abi = verify_landlock_is_enabled();
	if (abi < 6)
		tst_brk(TCONF, "LANDLOCK_SCOPE_ABSTRACT_UNIX_SOCKET is unsupported on ABI < 6");
}

static struct tst_test test = {
	.test_all = run,
	.setup = setup,
	.needs_root = 1,
	.forks_child = 1,
	.needs_checkpoints = 1,
	.test_variants = 3,
	.bufs = (struct tst_buffers []) {
		{&ruleset_attr, .size = sizeof(struct tst_landlock_ruleset_attr_abi6)},
		{},
	},
	.caps = (struct tst_cap []) {
		TST_CAP(TST_CAP_REQ, CAP_SYS_ADMIN),
		{}
	},
};
