// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2018 Google, Inc.
 */

/*
 * Regression test for commit 966031f340185 ("n_tty: fix EXTPROC vs ICANON
 * interaction with TIOCINQ (aka FIONREAD)").  The test reproduces a hang
 * (infinite loop in the kernel) after a pseudoterminal is put in both canonical
 * (ICANON) and external processing (EXTPROC) mode, some data is written to the
 * master and read from the slave, and the FIONREAD ioctl is called on the
 * slave.  This is simplified from a syzkaller-generated reproducer.
 */

#define _GNU_SOURCE
#include <stdlib.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <termios.h>

#include "tst_test.h"
#include "lapi/termbits.h"

static void do_test(void)
{
	struct termios io;
	int ptmx, pts;
	char c = 'A';
	int nbytes;

	ptmx = SAFE_OPEN("/dev/ptmx", O_WRONLY);

	if (tcgetattr(ptmx, &io) != 0)
		tst_brk(TBROK | TERRNO, "tcgetattr() failed");

	io.c_lflag = EXTPROC | ICANON;

	TEST(tcsetattr(ptmx, TCSANOW, &io));
	if (TST_RET == -1) {
		if (TST_ERR == EINVAL)
			tst_brk(TCONF, "tcsetattr(, , EXTPROC | ICANON) is not supported");
		tst_brk(TBROK | TERRNO, "tcsetattr() failed");
	}

	if (unlockpt(ptmx) != 0)
		tst_brk(TBROK | TERRNO, "unlockpt() failed");

	pts = SAFE_OPEN(ptsname(ptmx), O_RDONLY);
	/* write newline to ptmx to avoid read() on pts to block */
	SAFE_WRITE(1, ptmx, "A\n", 2);
	SAFE_READ(1, pts, &c, 1);

	tst_res(TINFO, "Calling FIONREAD, this will hang in n_tty_ioctl() if the bug is present...");
	SAFE_IOCTL(pts, FIONREAD, &nbytes);

	SAFE_CLOSE(ptmx);
	SAFE_CLOSE(pts);

	tst_res(TPASS, "Got to the end without hanging");
}

static struct tst_test test = {
	.test_all = do_test,
	.tags = (const struct tst_tag[]) {
		{"linux-git", "966031f34018"},
		{}
	}
};
