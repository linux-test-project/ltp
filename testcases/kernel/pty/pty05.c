// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2020 SUSE LLC <mdoucha@suse.cz>
 */

/*
 * CVE-2017-2636
 *
 * Check for race between flush_tx_queue() and n_hdlc_send_frames(). Kernel
 * crash fixed in:
 *
 *  commit 82f2341c94d270421f383641b7cd670e474db56b
 *  Author: Alexander Popov <alex.popov@linux.com>
 *  Date:   Tue Feb 28 19:54:40 2017 +0300
 *
 *  tty: n_hdlc: get rid of racy n_hdlc.tbuf
 */

#define _GNU_SOURCE
#include <termios.h>
#include "lapi/ioctl.h"
#include "lapi/tty.h"

#include "tst_test.h"
#include "tst_fuzzy_sync.h"

#define BUF_SIZE 1

static struct tst_fzsync_pair fzsync_pair;
static volatile int ptmx = -1;
static char buf[BUF_SIZE];

static void setup(void)
{
	fzsync_pair.exec_loops = 100000;
	tst_fzsync_pair_init(&fzsync_pair);
}

static void *thread_run(void *arg)
{
	while (tst_fzsync_run_b(&fzsync_pair)) {
		tst_fzsync_start_race_b(&fzsync_pair);
		ioctl(ptmx, TCFLSH, TCIOFLUSH);
		tst_fzsync_end_race_b(&fzsync_pair);
	}

	return arg;
}

static void run(void)
{
	int ldisc = N_HDLC;

	tst_fzsync_pair_reset(&fzsync_pair, thread_run);

	while (tst_fzsync_run_a(&fzsync_pair)) {
		ptmx = SAFE_OPEN("/dev/ptmx", O_RDWR);
		TEST(ioctl(ptmx, TIOCSETD, &ldisc));

		if (TST_RET == -1 && TST_ERR == EINVAL) {
			tst_brk(TCONF, "HDLC line discipline not available");
		} else if (TST_RET == -1) {
			tst_brk(TBROK | TTERRNO, "Cannot set line discipline");
		} else if (TST_RET != 0) {
			tst_brk(TBROK | TTERRNO,
				"Invalid ioctl() return value %ld", TST_RET);
		}

		SAFE_IOCTL(ptmx, TCXONC, TCOOFF);
		SAFE_WRITE(SAFE_WRITE_ALL, ptmx, buf, BUF_SIZE);

		tst_fzsync_start_race_a(&fzsync_pair);
		ioctl(ptmx, TCXONC, TCOON);
		tst_fzsync_end_race_a(&fzsync_pair);

		SAFE_CLOSE(ptmx);

		if (tst_taint_check()) {
			tst_res(TFAIL, "Kernel is vulnerable");
			return;
		}
	}

	tst_res(TPASS, "Nothing bad happened, probably");
}

static void cleanup(void)
{
	tst_fzsync_pair_cleanup(&fzsync_pair);

	if (ptmx >= 0)
		SAFE_CLOSE(ptmx);
}

static struct tst_test test = {
	.test_all = run,
	.setup = setup,
	.cleanup = cleanup,
	.taint_check = TST_TAINT_W | TST_TAINT_D,
	.runtime = 150,
	.tags = (const struct tst_tag[]) {
		{"linux-git", "82f2341c94d27"},
		{"CVE", "2017-2636"},
		{}
	}
};
