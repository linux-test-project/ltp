/*
 * Copyright (c) 2018 Google, Inc.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program, if not, see <http://www.gnu.org/licenses/>.
 */

/*
 * Regression test for commit 966031f340185 ("n_tty: fix EXTPROC vs ICANON
 * interaction with TIOCINQ (aka FIONREAD)").  The test reproduces a hang
 * (infinite loop in the kernel) after a pseudoterminal is put in both canonical
 * (ICANON) and external processing (EXTPROC) mode, some data is written to the
 * master and read from the slave, and the FIONREAD ioctl is called on the
 * slave.  This is simplified from a syzkaller-generated reproducer.
 */

#include <stdlib.h>
#include <errno.h>
#include <termio.h>

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
};
