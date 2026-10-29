// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines  Corp., 2002
 * Copyright (c) 2020 Petr Vorel <pvorel@suse.cz>
 * Copyright (C) 2024 SUSE LLC Andrea Cervesato <andrea.cervesato@suse.com>
 */

/*\
 * Verify that it's possible to open a pseudo-terminal via /dev/ptmx, obtain a
 * slave device and configure termos/termios ioctls.
 */

#define _GNU_SOURCE

#include <termios.h>
#include "common.h"
#include "lapi/ioctl.h"

static int masterfd = -1;

static void run(void)
{
	int slavefd;
	struct termio termio;
	struct termios termios;

	slavefd = open_slave(masterfd);

	TST_EXP_PASS(ioctl(slavefd, TCGETS, &termios));
	TST_EXP_PASS(ioctl(slavefd, TCSETS, &termios));
	TST_EXP_PASS(ioctl(slavefd, TCSETSW, &termios));
	TST_EXP_PASS(ioctl(slavefd, TCSETSF, &termios));
	TST_EXP_PASS(ioctl(slavefd, TCSETS, &termios));
	TST_EXP_PASS(ioctl(slavefd, TCGETA, &termio));
	TST_EXP_PASS(ioctl(slavefd, TCSETA, &termio));
	TST_EXP_PASS(ioctl(slavefd, TCSETAW, &termio));
	TST_EXP_PASS(ioctl(slavefd, TCSETAF, &termio));

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
