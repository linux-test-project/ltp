// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) Wipro Technologies Ltd, 2002.  All Rights Reserved.
 * Copyright (c) 2018 Xiao Yang <yangx.jy@cn.fujitsu.com>
 */

/*
 * AUTHOR: Madhu T L <madhu.tarikere@wipro.com>
 *
 * DESCRIPTION
 * Verify that,
 * 1. delete_module(2) returns -1 and sets errno to ENOENT for nonexistent
 *    module entry.
 * 2. delete_module(2) returns -1 and sets errno to EFAULT, if
 *    module name parameter is outside program's accessible address space.
 * 3. delete_module(2) returns -1 and sets errno to EPERM, if effective
 *    user id of the caller is not superuser.
 */

#include <errno.h>
#include <pwd.h>
#include <stdio.h>
#include <string.h>

#include "tst_test.h"
#include "lapi/syscalls.h"

#define BASEMODNAME	"dummy"
#define LONGMODNAMECHAR	'm'

/*
 * From kernel internal headers: include/linux/module.h
 * include/linux/moduleparam.h
 */
#define MODULE_NAME_LEN	(64 - sizeof(unsigned long))

static struct passwd *ltpuser;
static char longmodname[MODULE_NAME_LEN];
static char modname[20];

static struct test_case_t {
	char *modname;
	int experrno;
	char *desc;
	/* 1: nobody_user 0: root_user */
	int nobody_user;
} tdat[] = {
	{ modname, ENOENT, "nonexistent module", 0},
	{ "", ENOENT, "null terminated module name", 0},
	{ NULL, EFAULT, "module name outside program's accessible address space", 0},
	{ longmodname, ENOENT, "long module name", 0},
	{ modname, EPERM, "non-superuser", 1},
};

static void do_delete_module(unsigned int n)
{
	struct test_case_t *tc = &tdat[n];

	if (!tc->modname)
		tc->modname = tst_get_bad_addr(NULL);

	if (tc->nobody_user)
		SAFE_SETEUID(ltpuser->pw_uid);

	tst_res(TINFO, "test %s", tc->desc);
	TEST(tst_syscall(__NR_delete_module, tc->modname, 0));
	if (TST_RET != -1) {
		tst_res(TFAIL, "delete_module() succeeded unexpectedly");
	} else if (TST_ERR == tc->experrno) {
		tst_res(TPASS | TTERRNO, "delete_module() failed as expected");
	} else {
		tst_res(TFAIL | TTERRNO, "delete_module() failed unexpectedly;"
			" expected: %s", tst_strerrno(tc->experrno));
	}

	if (tc->nobody_user)
		SAFE_SETEUID(0);
}

static void setup(void)
{
	ltpuser = SAFE_GETPWNAM("nobody");

	/* Initialize longmodname to LONGMODNAMECHAR character */
	memset(longmodname, LONGMODNAMECHAR, MODULE_NAME_LEN - 1);

	/* Get unique module name for each child process */
	sprintf(modname, "%s_%d", BASEMODNAME, getpid());
}

static struct tst_test test = {
	.tcnt = ARRAY_SIZE(tdat),
	.needs_root = 1,
	.setup = setup,
	.test = do_delete_module,
};
