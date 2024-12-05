// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines  Corp., 2002
 * Copyright (c) 2020 Petr Vorel <pvorel@suse.cz>
 * Copyright (C) 2024 SUSE LLC Andrea Cervesato <andrea.cervesato@suse.com>
 */

/*\
 * [Description]
 *
 * Verify that it's possible to open a pseudo-terminal via /dev/ptmx, to obtain
 * a slave device and to set baudrate to B0 (which means hang up).
 */

#define _GNU_SOURCE

#include <termios.h>
#include "common.h"

static int masterfd = -1;

static void run(void)
{
	int slavefd;
	struct termios termios;

	slavefd = open_slave(masterfd);

	TST_EXP_PASS(ioctl(slavefd, TCGETS, &termios));
	termios.c_cflag &= ~CBAUD;
	termios.c_cflag |= B0 & CBAUD;
	TST_EXP_PASS(ioctl(slavefd, TCSETS, &termios));

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
