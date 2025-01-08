// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2020 SUSE LLC <mdoucha@suse.cz>
 */

/*
 * CVE-2019-8912
 *
 * Check for possible use-after-free in sockfs_setattr() on AF_ALG socket
 * closed by dup2() or dup3(). Unlike regular close(), dup*() syscalls don't
 * set sock->sk = NULL after closing the socket. Racing fchownat() against
 * dup2() may then result in sockfs_setattr() using the stale pointer and
 * writing into a block of released memory that may have been reused in the
 * mean time.
 *
 * The race window is small and it's hard to trigger a kernel crash but
 * fchownat() will return ENOENT as it should only when the bug is not
 * present. Race fixed specifically for af_alg in:
 *
 *  commit 9060cb719e61b685ec0102574e10337fa5f445ea
 *  Author: Mao Wenan <maowenan@huawei.com>
 *  Date:   Mon Feb 18 10:44:44 2019 +0800
 *
 *  net: crypto set sk to NULL when af_alg_release.
 *
 * It was observed that the same bug is present on many other
 * protocols. A more general fix is in:
 *
 *  commit ff7b11aa481f682e0e9711abfeb7d03f5cd612bf
 *  Author: Eric Biggers <ebiggers@google.com>
 *  Date:   Thu Feb 21 14:13:56 2019 -0800
 *
 *  net: socket: set sock->sk to NULL after calling proto_ops::release()
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pwd.h>
#include "lapi/fcntl.h"

#include "tst_test.h"
#include "tst_af_alg.h"
#include "tst_fuzzy_sync.h"
#include "tst_taint.h"

static int fd = -1, sock = -1;
static int uid, gid;
static struct tst_fzsync_pair fzsync_pair;

static void setup(void)
{
	uid = getuid();
	gid = getgid();

	fd = SAFE_OPEN("tmpfile", O_RDWR | O_CREAT, 0644);

	tst_fzsync_pair_init(&fzsync_pair);
}

static void *thread_run(void *arg)
{
	while (tst_fzsync_run_b(&fzsync_pair)) {
		tst_fzsync_start_race_b(&fzsync_pair);
		dup2(fd, sock);
		tst_fzsync_end_race_b(&fzsync_pair);
	}

	return arg;
}

static void run(void)
{
	tst_fzsync_pair_reset(&fzsync_pair, thread_run);

	while (tst_fzsync_run_a(&fzsync_pair)) {
		sock = tst_alg_setup_reqfd("hash", "sha1", NULL, 0);
		tst_fzsync_start_race_a(&fzsync_pair);
		TEST(fchownat(sock, "", uid, gid, AT_EMPTY_PATH));
		tst_fzsync_end_race_a(&fzsync_pair);
		SAFE_CLOSE(sock);

		if (tst_taint_check()) {
			tst_res(TFAIL, "Kernel is vulnerable");
			return;
		}

		if (TST_RET == -1 && TST_ERR == EBADF) {
			tst_fzsync_pair_add_bias(&fzsync_pair, 1);
			continue;
		}

		if (TST_RET == -1 && TST_ERR == ENOENT) {
			tst_res(TPASS | TTERRNO,
				"fchownat() failed successfully");
			return;
		}

		if (TST_RET == -1) {
			tst_brk(TBROK | TTERRNO,
				"fchownat() failed unexpectedly");
		}

		if (TST_RET) {
			tst_brk(TBROK | TTERRNO,
				"Invalid fchownat() return value");
		}
	}

	tst_res(TFAIL, "fchownat() failed to fail, kernel may be vulnerable");
}

static void cleanup(void)
{
	tst_fzsync_pair_cleanup(&fzsync_pair);

	if (fd >= 0)
		SAFE_CLOSE(fd);
}

static struct tst_test test = {
	.needs_tmpdir = 1,
	.test_all = run,
	.setup = setup,
	.cleanup = cleanup,
	.min_kver = "4.10.0",
	.min_cpus = 2,
	.runtime = 150,
	.taint_check = TST_TAINT_W | TST_TAINT_D,
	.tags = (const struct tst_tag[]) {
		{"linux-git", "ff7b11aa481f"},
		{"linux-git", "9060cb719e61"},
		{"CVE", "2019-8912"},
		{}
	}
};
