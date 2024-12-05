// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines  Corp., 2002
 * Copyright (C) 2024 SUSE LLC Andrea Cervesato <andrea.cervesato@suse.com>
 */

/*\
 * [Description]
 *
 * Verify that write/read is properly working when master and slave
 * pseudo terminals communicate with each other.
 */

#define _GNU_SOURCE

#include "common.h"

#define STRING "Linux Test Project\n"

static size_t string_len;
static int masterfd = -1;
static int slavefd = -1;

static void run(void)
{
	char buf[BUFSIZ];

	tst_res(TINFO, "Send message to master and read it from slave");

	memset(buf, 0, BUFSIZ);
	SAFE_WRITE(SAFE_WRITE_ALL, masterfd, STRING, string_len);
	SAFE_READ(0, slavefd, buf, string_len);
	TST_EXP_EQ_STRN(STRING, buf, string_len - 1);

	tst_res(TINFO, "Send message to slave and read it from master");

	memset(buf, 0, BUFSIZ);
	SAFE_WRITE(SAFE_WRITE_ALL, slavefd, STRING, string_len);

	/* we need to write string_len + 1, because kernel converts newline
	 * into carriage return + newline
	 */
	SAFE_READ(0, masterfd, buf, string_len + 1);
	TST_EXP_EQ_STRN(STRING, buf, string_len - 1);
}

static void setup(void)
{
	masterfd = open_master();
	slavefd = open_slave(masterfd);

	string_len = strlen(STRING);
}

static void cleanup(void)
{
	if (masterfd != -1)
		SAFE_CLOSE(masterfd);

	if (slavefd != -1)
		SAFE_CLOSE(slavefd);
}

static struct tst_test test = {
	.test_all = run,
	.setup = setup,
	.cleanup = cleanup,
};
