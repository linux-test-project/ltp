// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) Wipro Technologies Ltd, 2002.  All Rights Reserved.
 * Copyright (c) 2020 Yang Xu <xuyang2018.jy@cn.fujitsu.com>
 * Author: Sowmya Adiga <sowmya.adiga@wipro.com>
 *
 * This is a error test for the socketcall(2) system call.
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

static unsigned long args_valid[3] = {PF_INET, SOCK_STREAM, 0};

struct test_case_t {
	int call;
	unsigned long *args;
	int exp_err;
	char *desc;
} TC[] = {
	{0, args_valid, EINVAL, "invalid call(<1)"},
	{21, args_valid, EINVAL, "invalid call(>20)"},
	{SYS_SOCKET, NULL, EFAULT, "invalid args address"},
};

static void verify_socketcall(unsigned int i)
{
	tst_res(TINFO, "%s", TC[i].desc);

	TEST(tst_syscall(__NR_socketcall, TC[i].call, TC[i].args));
	if (TST_RET != -1) {
		tst_res(TFAIL, "socketcall() succeeded unexpectedly");
		return;
	}
	if (TST_ERR == TC[i].exp_err)
		tst_res(TPASS | TTERRNO, "socketcall() failed as expected ");
	else
		tst_res(TFAIL | TTERRNO, "socketcall fail expected %s got", tst_strerrno(TC[i].exp_err));
}

static void setup(void)
{
	unsigned int i;

	for (i = 0; i < ARRAY_SIZE(TC); i++) {
		if (!TC[i].args)
			TC[i].args = tst_get_bad_addr(NULL);
	}
}

static struct tst_test test = {
	.setup = setup,
	.test = verify_socketcall,
	.tcnt = ARRAY_SIZE(TC),
};

