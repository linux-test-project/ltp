// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2021 SUSE LLC <rpalethorpe@suse.com>
 */

/*\
 * Reproducer of CVE-2021-26708
 *
 * Based on POC https://github.com/jordan9001/vsock_poc.
 * Fuzzy Sync has been substituted for userfaultfd.
 *
 * Fixed by: c518adafa39f ("vsock: fix the race conditions in multi-transport support")
 *
 * Fixes: c0cfa2d8a788fcf4 ("vsock: add multi-transports support")
 *
 * Note that in many testing environments this will reproduce the race
 * silently. For the test to produce visible errors the loopback
 * transport should be registered, but not the g2h or h2g transports.
 *
 * One way to do this is to remove CONFIG_VIRTIO_VSOCKETS in the guest
 * or CONFIG_VHOST_VSOCK on the host. Or just unload the
 * modules. Alternatively run the test on a bare metal host which has
 * never started a VM.
 */

#include "config.h"
#include "tst_test.h"

#if HAVE_LINUX_VM_SOCKETS_H
#  include "tst_fuzzy_sync.h"
#  include "lapi/vm_sockets.h"

static struct tst_fzsync_pair pair;
static int vsock = -1;

static void *writer(LTP_ATTRIBUTE_UNUSED void *unused)
{
	const uint64_t b_buflen = 0x4141;

	while (tst_fzsync_run_b(&pair)) {
		tst_fzsync_start_race_b(&pair);
		SAFE_SETSOCKOPT(vsock, AF_VSOCK,
				SO_VM_SOCKETS_BUFFER_SIZE,
				&b_buflen, sizeof(b_buflen));
		tst_fzsync_end_race_b(&pair);
	}


	return NULL;
}

static void run(void)
{
	struct sockaddr_vm addr = { 0 };
	const struct timeval timeout = { 0, 1 };
	const uint64_t a_buflen = 0x4140;

	vsock = SAFE_SOCKET(AF_VSOCK, SOCK_STREAM, 0);
	SAFE_SETSOCKOPT(vsock, AF_VSOCK, SO_VM_SOCKETS_CONNECT_TIMEOUT,
			&timeout, sizeof(timeout));

	tst_res(TINFO, "Colliding transport change and setsockopt");
	tst_fzsync_pair_reset(&pair, writer);
	while (tst_fzsync_run_a(&pair)) {

		addr.svm_family = AF_VSOCK;
		addr.svm_port = 1234;
		addr.svm_cid = VMADDR_CID_LOCAL;

		if (!connect(vsock, (struct sockaddr *)&addr, sizeof(addr)))
			tst_brk(TCONF, "Connected to something on VSOCK loopback");

		if (errno == ENODEV)
			tst_brk(TCONF | TERRNO, "No loopback transport");

		SAFE_SETSOCKOPT(vsock, AF_VSOCK,
				SO_VM_SOCKETS_BUFFER_SIZE,
				&a_buflen, sizeof(a_buflen));

		addr.svm_family = AF_VSOCK;
		addr.svm_port = 5678;
		addr.svm_cid = VMADDR_CID_HOST + 3;

		tst_fzsync_start_race_a(&pair);
		TEST(connect(vsock, (struct sockaddr *)&addr, sizeof(addr)));
		tst_fzsync_end_race_a(&pair);

		if (!TST_RET) {
			tst_brk(TCONF,
				"g2h or h2g transport exists and we connected to something");
		}
	}

	SAFE_CLOSE(vsock);
	tst_res(TPASS, "Nothing bad happened, probably.");
}

static void cleanup(void)
{
	tst_fzsync_pair_cleanup(&pair);
}

static void setup(void)
{
	tst_fzsync_pair_init(&pair);
}

static struct tst_test test = {
	.test_all = run,
	.setup = setup,
	.cleanup = cleanup,
	.taint_check = TST_TAINT_W | TST_TAINT_D,
	.runtime = 60,
	.needs_kconfigs = (const char *[]) {
		"CONFIG_VSOCKETS_LOOPBACK",
		NULL
	},
	.tags = (const struct tst_tag[]) {
		{"linux-git", "c518adafa39f"},
		{"CVE", "CVE-2021-26708"},
		{}
	},
};

#else

TST_TEST_TCONF("No linux/vm_sockets.h");

#endif
