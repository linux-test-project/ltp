// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) Wipro Technologies Ltd, 2002.  All Rights Reserved.
 * AUTHOR: Aniruddha Marathe <aniruddha.marathe@wipro.com>
 */

/*\
 * [Description]
 * This is a Phase I test for the reboot(2) system call.
 * It is intended to provide a limited exposure of the system call.
 *
 * [Algorithm]
 * 1) Two test cases for two flag values
 * 2) Execute system call
 * 3) Check return code, if system call failed (return=-1)
 * 4) Log the errno and Issue a FAIL message
 * 5) Otherwise, Issue a PASS message
 *
 * [Restrictions]
 * For lib4 and lib5 reboot(2) system call is implemented as
 * int reboot(int magic, int magic2, int flag, void *arg); This test case
 * is written for int reboot(int flag); which is implemented under glibc
 * Therefore this testcase may not work under libc4 and libc5 libraries.
 */

#include <unistd.h>
#include <sys/reboot.h>
#include <linux/reboot.h>
#include "tst_test.h"

static struct tcase {
	const char *option_message;
	int flag;
} tcases[] = {
	{"LINUX_REBOOT_CMD_CAD_ON", LINUX_REBOOT_CMD_CAD_ON,},
	{"LINUX_REBOOT_CMD_CAD_OFF", LINUX_REBOOT_CMD_CAD_OFF,},
};

static void run(unsigned int n)
{
	TST_EXP_PASS(reboot(tcases[n].flag),
		"reboot(%s)", tcases[n].option_message);
}

static struct tst_test test = {
	.needs_root = 1,
	.test = run,
	.tcnt = ARRAY_SIZE(tcases),
};
