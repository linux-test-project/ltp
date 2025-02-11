// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines  Corp., 2002
 * Copyright (c) 2020 Petr Vorel <pvorel@suse.cz>
 * Copyright (C) 2024 SUSE LLC Andrea Cervesato <andrea.cervesato@suse.com>
 */

/*\
 * Verify that it's possible to open a pseudo-terminal via /dev/ptmx, obtain a
 * slave device and set/get window size.
 */

#define _GNU_SOURCE

#include "common.h"

static int masterfd = -1;

static void run(void)
{
	int slavefd;
	struct winsize wsz;
	struct winsize wsz1 = { 24, 80, 5, 10 };
	struct winsize wsz2 = { 60, 100, 11, 777 };

	slavefd = open_slave(masterfd);

	TST_EXP_PASS(ioctl(masterfd, TIOCSWINSZ, &wsz1));
	TST_EXP_PASS(ioctl(slavefd, TIOCGWINSZ, &wsz));

	TST_EXP_EQ_LI(wsz.ws_row, wsz1.ws_row);
	TST_EXP_EQ_LI(wsz.ws_col, wsz1.ws_col);
	TST_EXP_EQ_LI(wsz.ws_xpixel, wsz1.ws_xpixel);
	TST_EXP_EQ_LI(wsz.ws_ypixel, wsz1.ws_ypixel);

	TST_EXP_PASS(ioctl(masterfd, TIOCGWINSZ, &wsz));

	TST_EXP_EQ_LI(wsz.ws_row, wsz1.ws_row);
	TST_EXP_EQ_LI(wsz.ws_col, wsz1.ws_col);
	TST_EXP_EQ_LI(wsz.ws_xpixel, wsz1.ws_xpixel);
	TST_EXP_EQ_LI(wsz.ws_ypixel, wsz1.ws_ypixel);

	TST_EXP_PASS(ioctl(slavefd, TIOCSWINSZ, &wsz2));
	TST_EXP_PASS(ioctl(slavefd, TIOCGWINSZ, &wsz));

	TST_EXP_EQ_LI(wsz.ws_row, wsz2.ws_row);
	TST_EXP_EQ_LI(wsz.ws_col, wsz2.ws_col);
	TST_EXP_EQ_LI(wsz.ws_xpixel, wsz2.ws_xpixel);
	TST_EXP_EQ_LI(wsz.ws_ypixel, wsz2.ws_ypixel);

	SAFE_CLOSE(slavefd);
}

static void setup(void)
{
	masterfd = open_master();
}

static void cleanup(void)
{
	if (masterfd != -1)
		SAFE_CLOSE(masterfd);
}

static struct tst_test test = {
	.test_all = run,
	.setup = setup,
	.cleanup = cleanup,
};
