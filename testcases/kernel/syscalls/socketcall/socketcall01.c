// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) Wipro Technologies Ltd, 2002.  All Rights Reserved.
 * Copyright (c) 2016 Cyril Hrubis <chrubis@suse.cz>
 * Copyright (c) Linux Test Project, 2017-2020
 * Author: Sowmya Adiga <sowmya.adiga@wipro.com>
 */

/*\
 * Basic test for the socketcall(2) raw syscall.
 *
 * Test creating TCP, UDP, raw socket and unix domain dgram.
 */

#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <linux/net.h>
#include <sys/un.h>
#include <netinet/in.h>

#include "tst_test.h"
#include "lapi/syscalls.h"

struct test_case_t {
	int call;
	unsigned long args[3];
	char *desc;
} TC[] = {
	{SYS_SOCKET, {PF_INET, SOCK_STREAM, 0}, "TCP stream"},
	{SYS_SOCKET, {PF_UNIX, SOCK_DGRAM, 0}, "unix domain dgram"},
	{SYS_SOCKET, {AF_INET, SOCK_RAW, 6}, "Raw socket"},
	{SYS_SOCKET, {PF_INET, SOCK_DGRAM, 17}, "UDP dgram"}
};

void verify_socketcall(unsigned int i)
{
	TEST(tst_syscall(__NR_socketcall, TC[i].call, TC[i].args));

	if (TST_RET < 0) {
		tst_res(TFAIL | TTERRNO, "socketcall() for %s failed with %li",
			TC[i].desc, TST_RET);
		return;
	}

	tst_res(TPASS, "socketcall() for %s", TC[i].desc);

	SAFE_CLOSE(TST_RET);
}

static struct tst_test test = {
	.test = verify_socketcall,
	.tcnt = ARRAY_SIZE(TC),
	.needs_root = 1,
};
