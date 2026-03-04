// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) Crackerjack Project., 2007
 * Copyright (c) Linux Test Project, 2024
 * Copyright (c) 2026 Cyril Hrubis <chrubis@suse.cz>
 */

/*\
 * Verify that :manpage:`uname(2)` succeeds and correctly identifies the system
 * as Linux.  The rest of the values, when possible, are matched againts the
 * strings from /proc/sys/kernel/. The only value we cannot easily assert is
 * the machine field which is the architecture the kernel was compiled for,
 * which would require special handling per each architecture.
 */

#define _GNU_SOURCE
#include <sys/utsname.h>
#include "tst_test.h"
#include "lapi/syscalls.h"

static struct utsname *name;

static void run(void)
{
	char proc_val[1024] = {};

	TST_EXP_PASS(tst_syscall(__NR_uname, name), "uname(name)");

	if (!TST_PASS)
		return;

	TST_EXP_EQ_STR(name->sysname, "Linux");

	SAFE_FILE_SCANF("/proc/sys/kernel/hostname", "%1023[^\n]", proc_val);
	TST_EXP_EQ_STR(name->nodename, proc_val);

	SAFE_FILE_SCANF("/proc/sys/kernel/osrelease", "%1023[^\n]", proc_val);
	TST_EXP_EQ_STR(name->release, proc_val);

	SAFE_FILE_SCANF("/proc/sys/kernel/version", "%1023[^\n]", proc_val);
	TST_EXP_EQ_STR(name->version, proc_val);

	SAFE_FILE_SCANF("/proc/sys/kernel/domainname", "%1023[^\n]", proc_val);
	TST_EXP_EQ_STR(name->domainname, proc_val);
}

static struct tst_test test = {
	.test_all = run,
	.bufs = (struct tst_buffers []) {
		{&name, .size = sizeof(*name)},
		{}
	}
};
