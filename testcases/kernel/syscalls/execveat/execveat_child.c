// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2018 MediaTek Inc.  All Rights Reserved.
 */

/*
 * execveat_child.c
 * dummy program which is used by execveat01.c and execveat03.c testcases
 */

#define TST_NO_DEFAULT_MAIN
#include "tst_test.h"

int main(void)
{
	tst_reinit();
	tst_res(TPASS, "execveat_child run as expected");
	return 0;
}
