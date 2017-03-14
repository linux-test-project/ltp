/*
 * Copyright (c) Wipro Technologies Ltd, 2002.  All Rights Reserved.
 *    AUTHOR : sowmya adiga<sowmya.adiga@wipro.com>
 * Copyright (c) 2016 Cyril Hrubis <chrubis@suse.cz>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it would be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 */
/*
 * This is a basic test for the socketcall(2) system call.
 */
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/syscall.h>
#include <linux/net.h>
#include <sys/un.h>
#include <netinet/in.h>

#include "tst_test.h"

#ifdef __NR_socketcall

#define socketcall(call, args) syscall(__NR_socketcall, call, args)

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
	TEST(socketcall(TC[i].call, TC[i].args));

	if (TEST_RETURN < 0) {
		tst_res(TFAIL | TTERRNO, "socketcall() for %s failed with %li",
			TC[i].desc, TEST_RETURN);
		return;
	}

	tst_res(TPASS, "socketcall() for %s", TC[i].desc);

	SAFE_CLOSE(TEST_RETURN);
}

static struct tst_test test = {
	.test = verify_socketcall,
	.tcnt = ARRAY_SIZE(TC),
	.needs_root = 1,
};

#else

TST_TEST_TCONF("The socketcall() syscall is not supported");

#endif
