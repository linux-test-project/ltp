// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2022 xiaoshoukui <xiaoshoukui@ruijie.com.cn>
 */

/*\
 * [Description]
 *
 * The VT_DISALLOCATE ioctl can free a virtual console while VT_RESIZEX ioctl is
 * still running, causing a use-after-free in vt_ioctl(). Because VT_RESIZEX ioctl
 * have not make sure vc_cons[i].d is not NULL after grabbing console_lock().
 *
 * Fixed by commit:
 *
 *  commit 6cd1ed50efd88261298577cd92a14f2768eddeeb
 *  Author: Eric Dumazet <edumazet@google.com>
 *  Date:   Mon Feb 10 11:07:21 2020 -0800
 *
 *    vt: vt_ioctl: fix race in VT_RESIZEX
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
#define MAX_NR_CONSOLES 63

static char tty_path[BUF_SIZE];
static int test_tty_port = 8;
static int fd = -1;
static struct tst_fzsync_pair fzp;

static struct vt_consize consize;
static unsigned short vt_active;

static void *open_close(void *unused)
{
	int i;

	while (tst_fzsync_run_b(&fzp)) {
		tst_fzsync_start_race_b(&fzp);
		for (i = test_tty_port; i < MAX_NR_CONSOLES; i++) {
			ioctl(fd, VT_ACTIVATE, i);
			ioctl(fd, VT_DISALLOCATE, i);
		}
		tst_fzsync_end_race_b(&fzp);
	}

	return unused;
}

static void do_test(void)
{

	tst_fzsync_pair_reset(&fzp, open_close);

	while (tst_fzsync_run_a(&fzp)) {
		tst_fzsync_start_race_a(&fzp);
		ioctl(fd, VT_RESIZEX, &consize);
		tst_fzsync_end_race_a(&fzp);
		if (tst_taint_check()) {
			tst_res(TFAIL, "Kernel is buggy");
			break;
		}
	}
	tst_res(TPASS, "Did not crash with VT_RESIZE");
}

static void setup(void)
{
	struct vt_stat stat;
	struct winsize wsize;

	sprintf(tty_path, "/dev/tty%d", test_tty_port);
	if (access(tty_path, F_OK))
		tst_brk(TCONF, "TTY (/dev/tty%d) under test not available in system", test_tty_port);

	fd = SAFE_OPEN(tty_path, O_RDWR);
	SAFE_IOCTL(fd, VT_GETSTATE, &stat);
	vt_active = stat.v_active;

	tst_res(TINFO, "Saving active console %i", vt_active);

	SAFE_IOCTL(fd, TIOCGWINSZ, &wsize);
	consize.v_rows = wsize.ws_row;
	consize.v_cols = wsize.ws_col;
	tst_fzsync_pair_init(&fzp);
}

static void cleanup(void)
{
	tst_fzsync_pair_cleanup(&fzp);

	if (fd >= 0) {
		tst_res(TINFO, "Restoring active console");
		SAFE_IOCTL(fd, VT_ACTIVATE, vt_active);
		SAFE_CLOSE(fd);
	}
}

static struct tst_test test = {
	.test_all = do_test,
	.setup = setup,
	.cleanup = cleanup,
	.needs_root = 1,
	.taint_check = TST_TAINT_W | TST_TAINT_D,
	.tags = (const struct tst_tag[]) {
		{ "linux-git", "6cd1ed50efd8"},
		{}
	}
};
