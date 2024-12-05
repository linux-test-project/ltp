// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines  Corp., 2002
 * Copyright (c) 2020 Petr Vorel <pvorel@suse.cz>
 * Copyright (C) 2024 SUSE LLC Andrea Cervesato <andrea.cervesato@suse.com>
 */

/*\
 * [Description]
 *
 * Verify that it's possible to open a pseudo-terminal via /dev/ptmx, obtain a
 * slave device and to send a break to both master and slave.
 */

#define _GNU_SOURCE

#include <termios.h>
#include "common.h"

static int masterfd = -1;

static void run(void)
{
	int slavefd;

	slavefd = open_slave(masterfd);

	TST_EXP_PASS(tcsendbreak(masterfd, 10));
	TST_EXP_PASS(tcsendbreak(slavefd, 10));

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
