// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines  Corp., 2002
 * Copyright (C) 2024 SUSE LLC Andrea Cervesato <andrea.cervesato@suse.com>
 */

/*\
 * Verify that slave pseudo-terminal fails reading/writing if master has been
 * closed.
 */

#define _GNU_SOURCE

#include "common.h"

static void run(void)
{
	int slavefd;
	int masterfd;
	char buf;

	masterfd = open_master();
	slavefd = open_slave(masterfd);

	tst_res(TINFO, "Closing master communication");
	SAFE_CLOSE(masterfd);

	TST_EXP_EQ_LI(read(slavefd, &buf, 1), 0);
	TST_EXP_FAIL(write(slavefd, &buf, 1), EIO);

	SAFE_CLOSE(slavefd);
}

static struct tst_test test = {
	.test_all = run,
};
