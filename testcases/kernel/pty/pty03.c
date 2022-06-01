// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2020 SUSE
 */

/*\
 * [Description]
 *
 * Test based on Syzkaller reproducer:
 * https://syzkaller.appspot.com/bug?id=680c24ff647dd7241998e19da52e8136d2fd3523
 *
 * The SLIP and SLCAN disciplines can have a race between write_wakeup and
 * close/hangup. This at least allows the netdev private data (tty->disc_data)
 * to be set to NULL or possibly freed while a transmit operation is being
 * added to a workqueue.
 *
 * If the race condition exists, then the most likely result of running this
 * is a null pointer dereference.
 *
 * Note that we must set the line discipline to "mouse" first which, for
 * whatever reason, results in tty_wakeup being called during the close
 * operation.
 *
 * We also test a selection of other line disciplines, but only SLIP and SLCAN
 * are known to have the problem.
 *
 * Fixed by commit from v5.5:
 * 0ace17d56824 ("can, slip: Protect tty->disc_data in write_wakeup and close with RCU")

 * This is also regression test for commit from v4.5-rc1:
 * dd42bf119714 ("tty: Prevent ldisc drivers from re-using stale tty fields")
 */

#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <termios.h>
#include "lapi/ioctl.h"

#include "tst_test.h"
#include "tst_safe_stdio.h"
#include "tst_fuzzy_sync.h"

struct ldisc_info {
	int n;
	char *name;
};

static struct ldisc_info ldiscs[] = {
	{2, "mouse"},

	{1, "SLIP"},
	{3, "Async PPP"},
	{5, "AX25/KISS"},
	{13, "HDLC"},
	{14, "Sync PPP"},
	{17, "SLCAN"},
	{18, "PPS"},
	{20, "CAIF"},
	{21, "GSM"}
};

static struct tst_fzsync_pair fzp;
static volatile int ptmx;

static void *hangup(void *unused)
{
	int i;

	while (tst_fzsync_run_b(&fzp)) {
		tst_fzsync_start_race_b(&fzp);
		for (i = 0; i < 10; i++) {
			if (tcflush(ptmx, TCIFLUSH))
				tst_brk(TBROK | TERRNO, "tcflush(ptmx, TCIFLUSH)");
		}
		tst_fzsync_end_race_b(&fzp);
	}

	return unused;
}

static int set_ldisc(int tty, struct ldisc_info *ldisc)
{
	TEST(ioctl(tty, TIOCSETD, &ldisc->n));

	if (!TST_RET)
		return 0;

	if (TST_ERR == EINVAL) {
		tst_res(TCONF | TTERRNO,
			"You don't appear to have the %s TTY line discipline",
			ldisc->name);
	} else {
		tst_res(TFAIL | TTERRNO,
			"Failed to set the %s line discipline", ldisc->name);
	}

	return 1;
}

static void do_test(unsigned int n)
{
	int pts;
	char pts_path[PATH_MAX];
	struct ldisc_info *ldisc = &ldiscs[n + 1];

	tst_res(TINFO, "Creating PTY with %s line discipline", ldisc->name);

	tst_fzsync_pair_reset(&fzp, hangup);
	while (tst_fzsync_run_a(&fzp)) {
		ptmx = SAFE_OPEN("/dev/ptmx", O_RDONLY);
		if (grantpt(ptmx))
			tst_brk(TBROK | TERRNO, "grantpt(ptmx)");
		if (unlockpt(ptmx))
			tst_brk(TBROK | TERRNO, "unlockpt(ptmx)");
		if (ptsname_r(ptmx, pts_path, sizeof(pts_path)))
			tst_brk(TBROK | TERRNO, "ptsname_r(ptmx, ...)");
		pts = SAFE_OPEN(pts_path, O_RDONLY);

		if (set_ldisc(pts, &ldiscs[0]))
			tst_brk(TCONF, "Need to set mouse discipline first");
		if (set_ldisc(pts, ldisc))
			return;

		tst_fzsync_start_race_a(&fzp);
		ioctl(pts, TIOCVHANGUP);
		tst_fzsync_end_race_a(&fzp);

		SAFE_CLOSE(pts);
		SAFE_CLOSE(ptmx);
	}

	tst_res(TPASS, "Did not crash with %s TTY discipline", ldisc->name);
}

static void setup(void)
{
	fzp.min_samples = 20;

	tst_fzsync_pair_init(&fzp);
}

static void cleanup(void)
{
	tst_fzsync_pair_cleanup(&fzp);
}

static struct tst_test test = {
	.test = do_test,
	.tcnt = 9,
	.setup = setup,
	.cleanup = cleanup,
	.needs_root = 1,
	.max_runtime = 30,
	.tags = (const struct tst_tag[]) {
		{"linux-git", "0ace17d568241"},
		{"CVE", "2020-14416"},
		{"linux-git", "dd42bf1197144"},
		{}
	}
};
