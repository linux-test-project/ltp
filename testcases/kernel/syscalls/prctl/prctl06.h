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

void check_no_new_privs(int val, char *name, int flag)
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
	if (flag)
		TST_ASSERT_FILE_INT(PROC_STATUS, "NoNewPrivs:", val);
}

#endif
