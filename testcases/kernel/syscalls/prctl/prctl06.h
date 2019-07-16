// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2019 FUJITSU LIMITED. All rights reserved.
 * Author: Yang Xu <xuyang2018.jy@cn.fujitsu.com>
 */
#ifndef PRCTL06_H
#define PRCTL06_H

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/prctl.h>
#include <pwd.h>
#include <sys/types.h>
#include <unistd.h>
#include <lapi/prctl.h>
#include "tst_test.h"

#define PROC_STATUS        "/proc/self/status"
#define IPC_ENV_VAR        "LTP_IPC_PATH"
#define MNTPOINT           "mntpoint"
#define TESTBIN            "prctl06_execve"
#define TEST_REL_BIN_DIR   MNTPOINT"/"
#define BIN_PATH           MNTPOINT"/"TESTBIN
#define SUID_MODE          (S_ISUID|S_ISGID|S_IXUSR|S_IXGRP|S_IXOTH)

void check_proc_field(int val, char *name)
{
	static int flag = 1;
	int field = 0;

	if (!flag)
		return;

	TEST(FILE_LINES_SCANF(PROC_STATUS, "NoNewPrivs:%d", &field));
	if (TST_RET == 1) {
		tst_res(TCONF,
			"%s doesn't support NoNewPrivs field", PROC_STATUS);
		flag = 0;
		return;
	}
	if (val == field)
		tst_res(TPASS, "%s %s NoNewPrivs field expected %d got %d",
			name, PROC_STATUS, val, field);
	else
		tst_res(TFAIL, "%s %s NoNewPrivs field expected %d got %d",
			name, PROC_STATUS, val, field);
}

void check_no_new_privs(int val, char *name)
{
	TEST(prctl(PR_GET_NO_NEW_PRIVS, 0, 0, 0, 0));
	if (TST_RET == val)
		tst_res(TPASS,
			"%s prctl(PR_GET_NO_NEW_PRIVS) expected %d got %d",
			name, val, val);
	else
		tst_res(TFAIL,
			"%s prctl(PR_GET_NO_NEW_PRIVS) expected %d got %ld",
			name, val, TST_RET);

	check_proc_field(val, name);
}

#endif
