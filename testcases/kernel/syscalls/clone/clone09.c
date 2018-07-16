/*
 * Copyright (c) 2017 Oracle and/or its affiliates. All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it would be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#define _GNU_SOURCE
#include <sched.h>
#include <stdlib.h>
#include <errno.h>

#include "tst_test.h"
#include "clone_platform.h"
#include "lapi/syscalls.h"
#include "lapi/namespaces_constants.h"

static void *child_stack;
static int sysctl_net = -1;
static int sysctl_net_new = -1;
static const char sysctl_path[] = "/proc/sys/net/ipv4/conf/lo/tag";
static const char sysctl_path_def[] = "/proc/sys/net/ipv4/conf/default/tag";
static int flags = CLONE_NEWNET | CLONE_VM | SIGCHLD;

static void setup(void)
{
	child_stack = SAFE_MALLOC(CHILD_STACK_SIZE);
}

static void cleanup(void)
{
	if (sysctl_net != -1)
		SAFE_FILE_PRINTF(sysctl_path, "%d", sysctl_net);

	free(child_stack);
}

static int newnet(void *arg LTP_ATTRIBUTE_UNUSED)
{
	SAFE_FILE_SCANF(sysctl_path, "%d", &sysctl_net_new);
	tst_syscall(__NR_exit, 0);
	return 0;
}

static long clone_child(void)
{
	TEST(ltp_clone(flags, newnet, NULL, CHILD_STACK_SIZE, child_stack));

	if (TST_RET == -1 && TST_ERR == EINVAL)
		tst_brk(TCONF, "CONFIG_NET_NS was disabled");

	if (TST_RET == -1)
		tst_brk(TBROK | TTERRNO, "clone(CLONE_NEWNET) failed");

	return TST_RET;
}

static void do_test(void)
{
	int def_val;

	tst_res(TINFO, "create clone in a new netns with 'CLONE_NEWNET' flag");

	SAFE_FILE_SCANF(sysctl_path, "%d", &sysctl_net);
	SAFE_FILE_PRINTF(sysctl_path, "%d", sysctl_net + 1);

	clone_child();
	tst_reap_children();

	if (sysctl_net_new == (sysctl_net + 1)) {
		tst_res(TFAIL, "sysctl params equal: %s=%d",
			sysctl_path, sysctl_net_new);
	}

	SAFE_FILE_SCANF(sysctl_path_def, "%d", &def_val);

	if (sysctl_net_new != def_val) {
		tst_res(TFAIL, "netns param init to non-default value %d",
			sysctl_net_new);
	}

	/* restore previous value */
	SAFE_FILE_PRINTF(sysctl_path, "%d", sysctl_net);

	tst_res(TPASS, "sysctl params differ in new netns");
}

static struct tst_test test = {
	.test_all = do_test,
	.setup = setup,
	.cleanup = cleanup,
	.needs_root = 1,
	.min_kver = "2.6.24",
};
