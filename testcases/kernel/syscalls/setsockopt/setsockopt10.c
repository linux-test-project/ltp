// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2023 SUSE LLC Richard Palethorpe <rpalethorpe@suse.com>
 */
/*\
 * [Description]
 *
 * Reproducer for CVE-2023-0461 which is an exploitable use-after-free
 * in a TLS socket. In fact it is exploitable in any User Level
 * Protocol (ULP) which does not clone its context when accepting a
 * connection.
 *
 * Because it does not clone the context, the child socket which is
 * created on accept has a pointer to the listening socket's
 * context. When the child is closed the parent's context is freed
 * while it still has a reference to it.
 *
 * TLS can only be added to a socket which is connected. Not listening
 * or disconnected, and a connected socket can not be set to
 * listening. So we have to connect the socket, add TLS, then
 * disconnect, then set it to listening.
 *
 * To my knowledge, setting a socket from open to disconnected
 * requires a trick; we have to "connect" to an unspecified
 * address. This could explain why the bug was not found earlier.
 *
 * The accepted fix was to disallow listening on sockets with a ULP
 * set which does not have a clone function.
 *
 * The test uses two processes, first the child acts as a server so
 * that the parent can create the TLS socket. Then the child connects
 * to the parent's TLS socket.
 *
 * When we try to listen on the parent, the current kernel should
 * return EINVAL. However if clone is implemented then this could
 * become a valid operation. It is also quite easy to crash the kernel
 * if we set some TLS options before doing a double free.
 *
 * commit 2c02d41d71f90a5168391b6a5f2954112ba2307c
 * Author: Paolo Abeni <pabeni@redhat.com>
 * Date:   Tue Jan 3 12:19:17 2023 +0100
 *
 *  net/ulp: prevent ULP without clone op from entering the LISTEN status
 */

#include "sched.h"
#include "tst_test.h"

#ifdef HAVE_LINUX_TLS_H

#include <linux/tls.h>
#include <netinet/in.h>

#include "lapi/sched.h"
#include "lapi/socket.h"
#include "lapi/tcp.h"
#include "tst_checkpoint.h"
#include "tst_net.h"
#include "tst_safe_net.h"
#include "tst_taint.h"

static struct tls12_crypto_info_aes_gcm_128 opts = {
	.info = {
		.version = TLS_1_2_VERSION,
		.cipher_type = TLS_CIPHER_AES_GCM_128,
	},
	.iv = { 'i', 'v' },
	.key = { 'k', 'e', 'y' },
	.salt = { 's', 'a', 'l', 't' },
	.rec_seq = { 'r', 'e', 'c', 's' },
};

static struct sockaddr_in tcp0_addr, tcp1_addr;
static const struct sockaddr unspec_addr = {
	.sa_family = AF_UNSPEC
};

static int tcp0_sk, tcp1_sk, tcp2_sk, tcp3_sk;

static void setup(void)
{
	tst_init_sockaddr_inet(&tcp0_addr, "127.0.0.1", 0x7c90);
	tst_init_sockaddr_inet(&tcp1_addr, "127.0.0.1", 0x7c91);
}

static void cleanup(void)
{
	if (tcp0_sk > 0)
		SAFE_CLOSE(tcp0_sk);
	if (tcp1_sk > 0)
		SAFE_CLOSE(tcp1_sk);
	if (tcp2_sk > 0)
		SAFE_CLOSE(tcp2_sk);
	if (tcp3_sk > 0)
		SAFE_CLOSE(tcp3_sk);
}

static void child(void)
{
	tst_res(TINFO, "child: Listen for tcp1 connection");
	tcp0_sk = SAFE_SOCKET(AF_INET, SOCK_STREAM, 0);
	SAFE_BIND(tcp0_sk, (struct sockaddr *)&tcp0_addr, sizeof(tcp0_addr));
	SAFE_LISTEN(tcp0_sk, 1);
	TST_CHECKPOINT_WAKE(0);

	tcp3_sk = SAFE_ACCEPT(tcp0_sk, NULL, 0);
	TST_CHECKPOINT_WAIT(1);
	SAFE_CLOSE(tcp3_sk);
	SAFE_CLOSE(tcp0_sk);

	tcp3_sk = SAFE_SOCKET(AF_INET, SOCK_STREAM, 0);
	TST_CHECKPOINT_WAIT(2);

	tst_res(TINFO, "child: connect for tcp2 connection");
	TEST(connect(tcp3_sk, (struct sockaddr *)&tcp1_addr, sizeof(tcp1_addr)));

	if (TST_RET == -1) {
		tst_res(TINFO | TTERRNO, "child: could not connect to tcp1");
		return;
	}

	TST_CHECKPOINT_WAIT(3);
}

static void run(void)
{
	const pid_t child_pid = SAFE_FORK();

	if (child_pid == 0) {
		child();
		return;
	}

	tcp1_sk = SAFE_SOCKET(AF_INET, SOCK_STREAM, 0);
	TST_CHECKPOINT_WAIT(0);

	tst_res(TINFO, "parent: Connect for tcp0 connection");
	SAFE_CONNECT(tcp1_sk, (struct sockaddr *)&tcp0_addr, sizeof(tcp0_addr));
	TEST(setsockopt(tcp1_sk, SOL_TCP, TCP_ULP, "tls", 3));

	if (TST_RET == -1 && TST_ERR == ENOENT)
		tst_brk(TCONF | TTERRNO, "parent: setsockopt failed: The TLS module is probably not loaded");
	else if (TST_RET == -1)
		tst_brk(TBROK | TTERRNO, "parent: setsockopt failed");

	SAFE_SETSOCKOPT(tcp1_sk, SOL_TLS, TLS_TX, &opts, sizeof(opts));
	TST_CHECKPOINT_WAKE(1);

	tst_res(TINFO, "parent: Disconnect by setting unspec address");
	SAFE_CONNECT(tcp1_sk, &unspec_addr, sizeof(unspec_addr));
	SAFE_BIND(tcp1_sk, (struct sockaddr *)&tcp1_addr, sizeof(tcp1_addr));

	TEST(listen(tcp1_sk, 1));

	if (TST_RET == -1) {
		if (TST_ERR == EINVAL)
			tst_res(TPASS | TTERRNO, "parent: Can't listen on disconnected TLS socket");
		else
			tst_res(TCONF | TTERRNO, "parent: Can't listen on disconnected TLS socket, but the errno is not EINVAL as expected");

		TST_CHECKPOINT_WAKE(2);
		tst_reap_children();
		return;
	}

	tst_res(TINFO, "parent: Can listen on disconnected TLS socket");
	TST_CHECKPOINT_WAKE(2);

	tcp2_sk = SAFE_ACCEPT(tcp1_sk, NULL, 0);
	SAFE_CLOSE(tcp2_sk);

	tst_res(TINFO, "parent: Attempting double free, because we set cipher options this should result in an crash");
	tst_flush();
	SAFE_CLOSE(tcp1_sk);

	TST_CHECKPOINT_WAKE(3);
	tst_reap_children();
	sched_yield();

	tst_res(TCONF, "parent: We're still here, maybe the kernel can clone the TLS-ULP context now?");
}

static struct tst_test test = {
	.setup = setup,
	.cleanup = cleanup,
	.test_all = run,
	.forks_child = 1,
	.needs_checkpoints = 1,
	.taint_check = TST_TAINT_W | TST_TAINT_D,
	.needs_kconfigs = (const char *[]) {
		"CONFIG_TLS",
		NULL
	},
	.tags = (const struct tst_tag[]) {
		{"linux-git", "2c02d41d71f90"},
		{"CVE", "2023-0461"},
		{}
	}
};

#else

TST_TEST_TCONF("linux/tls.h missing, we assume your system is too old");

#endif
