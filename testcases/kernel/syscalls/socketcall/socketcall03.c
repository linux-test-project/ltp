// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) Wipro Technologies Ltd, 2002.  All Rights Reserved.
 * Copyright (c) 2020 Yang Xu <xuyang2018.jy@cn.fujitsu.com>
 * Author: Sowmya Adiga <sowmya.adiga@wipro.com>
 *
 * This is a basic test for the socketcall(2) for bind(2) and listen(2).
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

static struct sockaddr_in si;

static void verify_socketcall(void)
{
	unsigned long args[3];
	int s = -1;

	s = SAFE_SOCKET(AF_INET, SOCK_STREAM, 6);
	args[0] = s;
	args[1] = (unsigned long)&si;
	args[2] = sizeof(si);

	TEST(tst_syscall(__NR_socketcall, SYS_BIND, args));
	if (TST_RET < 0)
		tst_res(TFAIL | TTERRNO, "socketcall() for bind call failed with %ld", TST_RET);
	else
		tst_res(TPASS, "socketcall() for bind call passed, returned %ld", TST_RET);

	args[1] = 1;
	args[2] = 0;

	TEST(tst_syscall(__NR_socketcall, SYS_LISTEN, args));
	if (TST_RET < 0)
		tst_res(TFAIL | TTERRNO, "socketcall() for listen call failed with %ld", TST_RET);
	else
		tst_res(TPASS, "socketcall() for listen call passed, returned %ld", TST_RET);

	SAFE_CLOSE(s);
}

static void setup(void)
{
	si.sin_family = AF_INET;
	si.sin_addr.s_addr = htons(INADDR_ANY);
	si.sin_port = 0;
}

static struct tst_test test = {
	.setup = setup,
	.test_all = verify_socketcall,
};
