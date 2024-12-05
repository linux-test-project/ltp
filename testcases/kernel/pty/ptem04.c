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
 * slave device and to check if it's possible to open it multiple times.
 */

#define _GNU_SOURCE

#include "common.h"

#define NUM_SLAVES 10

static int masterfd = -1;

static void run(void)
{
	int slavefd[NUM_SLAVES];

	for (int i = 0; i < NUM_SLAVES; i++)
		slavefd[i] = TST_EXP_FD(open_slave(masterfd));

	for (int i = 0; i < NUM_SLAVES; i++)
		SAFE_CLOSE(slavefd[i]);
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
