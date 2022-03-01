// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2022 xiaoshoukui <xiaoshoukui@ruijie.com.cn>
 */

/*\
 * [Description]
 *
 * Test based on Syzkaller reproducer:
 * https://syzkaller.appspot.com/bug?extid=522643ab5729b0421998
 *
 * The VT_DISALLOCATE ioctl can free a virtual console while tty_release() is
 * still running, causing a use-after-free in con_shutdown(). This occurs
 * because VT_DISALLOCATE only considers a virtual console to be in-use if it
 * has a tty_struct with count > 0. But actually when count == 0, the tty is
 * still in the process of being closed.
 *
 * Fixed by commit:
 *
 *  commit ca4463bf8438b403596edd0ec961ca0d4fbe0220
 *  Author: Eric Biggers <ebiggers@google.com>
 *  Date:   Sat Mar 21 20:43:04 2020 -0700
 *
 *    vt: vt_ioctl: fix VT_DISALLOCATE freeing in-use virtual console
 */

#define _GNU_SOURCE

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <termios.h>
#include <linux/vt.h>
#include "lapi/ioctl.h"

#include "tst_test.h"
#include "tst_safe_stdio.h"
#include "tst_fuzzy_sync.h"

#define BUF_SIZE 256
static char tty_path_a[BUF_SIZE];
static char tty_path_b[BUF_SIZE];
static int test_tty_port = 8;
static struct tst_fzsync_pair fzp;

static void *open_close(void *unused)
{
	sprintf(tty_path_b, "/dev/tty%d", test_tty_port);

	while (tst_fzsync_run_b(&fzp)) {
		tst_fzsync_start_race_b(&fzp);
		int fd = SAFE_OPEN(tty_path_b, O_RDWR);

		SAFE_CLOSE(fd);
		tst_fzsync_end_race_b(&fzp);
	}

	return unused;
}

static void do_test(void)
{
	sprintf(tty_path_a, "/dev/tty%d", test_tty_port + 1);
	int fd = SAFE_OPEN(tty_path_a, O_RDWR);

	tst_fzsync_pair_reset(&fzp, open_close);

	while (tst_fzsync_run_a(&fzp)) {
		tst_fzsync_start_race_a(&fzp);
		ioctl(fd, VT_DISALLOCATE, test_tty_port);
		tst_fzsync_end_race_a(&fzp);
		if (tst_taint_check()) {
			tst_res(TFAIL, "Kernel is vulnerable");
			return;
		}
	}
	SAFE_CLOSE(fd);
	tst_res(TPASS, "Did not crash with VT_DISALLOCATE");
}

static void setup(void)
{
	tst_fzsync_pair_init(&fzp);
}

static void cleanup(void)
{
	tst_fzsync_pair_cleanup(&fzp);
}

static struct tst_test test = {
	.test_all = do_test,
	.setup = setup,
	.cleanup = cleanup,
	.needs_root = 1,
	.taint_check = TST_TAINT_W | TST_TAINT_D,
	.tags = (const struct tst_tag[]) {
	    { "linux-git", "ca4463bf8438"},
	    {}
	}
};
