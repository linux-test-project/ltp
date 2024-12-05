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
 * a master + slave pair and to open them multiple times.
 */

#define _GNU_SOURCE

#include "common.h"

#define NUM_OPENS 10

static int masterfd[NUM_OPENS];

static void run(void)
{
	int slavefd[NUM_OPENS];

	for (int i = 0; i < NUM_OPENS; i++)
		slavefd[i] = TST_EXP_FD(open_slave(masterfd[i]));

	for (int i = 0; i < NUM_OPENS; i++)
		SAFE_CLOSE(slavefd[i]);
}

static void setup(void)
{
	for (int i = 0; i < NUM_OPENS; i++)
		masterfd[i] = -1;

	for (int i = 0; i < NUM_OPENS; i++)
		masterfd[i] = open_master();
}

static void cleanup(void)
{
	for (int i = 0; i < NUM_OPENS; i++) {
		if (masterfd[i] != -1)
			SAFE_CLOSE(masterfd[i]);
	}
}

static struct tst_test test = {
	.test_all = run,
	.setup = setup,
	.cleanup = cleanup,
};
