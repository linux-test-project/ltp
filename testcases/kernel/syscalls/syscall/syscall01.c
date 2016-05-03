/*
 * Copyright (c) International Business Machines  Corp., 2002
 * 01/02/2003	Port to LTP avenkat@us.ibm.com
 * 06/30/2001	Port to Linux nsharoff@us.ibm.com
 *
 * This program is free software;  you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY;  without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
 * the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program;  if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

/*
 * Basic test for syscall().
 */

#define _GNU_SOURCE
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/types.h>

#include "tst_test.h"
#include "linux_syscall_numbers.h"

static void verify_getpid(void)
{
	pid_t p1, p2;

	p1 = getpid();
	p2 = syscall(SYS_getpid);

	if (p1 == p2) {
		tst_res(TPASS, "getpid() == syscall(SYS_getpid)");
	} else {
		tst_res(TFAIL, "getpid() = %i, syscall(SYS_getpid) = %i",
			p1, p2);
	}
}

static void verify_getuid(void)
{
	uid_t u1, u2;

	u1 = getuid();
	u2 = syscall(SYS_getuid);

	if (u1 == u2) {
		tst_res(TPASS, "getuid() == syscall(SYS_getuid)");
	} else {
		tst_res(TFAIL, "getuid() = %i, syscall(SYS_getuid) = %i",
			u1, u2);
	}
}

static void verify_getgid(void)
{
	gid_t g1, g2;

	g1 = getgid();
	g2 = syscall(SYS_getgid);

	if (g1 == g2) {
		tst_res(TPASS, "getgid() == syscall(SYS_getgid)");
	} else {
		tst_res(TFAIL, "getgid() = %i, syscall(SYS_getgid) = %i",
			g1, g2);
	}
}


static void (*tcases[])(void) = {
	verify_getpid,
	verify_getuid,
	verify_getgid,
};

static void verify_syscall(unsigned int n)
{
	tcases[n]();
}

static struct tst_test test = {
	.tid = "syscall01",
	.test = verify_syscall,
	.tcnt = ARRAY_SIZE(tcases),
};

