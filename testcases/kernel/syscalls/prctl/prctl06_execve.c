// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2019 FUJITSU LIMITED. All rights reserved.
 * Author: Yang Xu <xuyang2018.jy@cn.fujitsu.com>
 *
 * dummy program which is used by prctl06 testcase
 */
#define TST_NO_DEFAULT_MAIN
#include "prctl06.h"


int main(int argc, char **argv)
{
	struct passwd *pw;
	int proc_flag;

	pw = SAFE_GETPWNAM("nobody");

	tst_reinit();
	if (argc != 3)
		tst_brk(TFAIL, "argc is %d, expected 3", argc);

	if (!strcmp(argv[2], "Yes"))
		proc_flag = 1;
	else
		proc_flag = 0;
	check_no_new_privs(1, argv[1], proc_flag);

	TEST(getegid());
	if (TST_RET == 0)
		tst_res(TFAIL,
			"%s getegid() returns 0 unexpectedly, it gains root privileges",
			argv[1]);
	if (TST_RET == pw->pw_gid)
		tst_res(TPASS,
			"%s getegid() returns nobody, it doesn't gain root privileges",
			argv[1]);

	TEST(geteuid());
	if (TST_RET == 0)
		tst_res(TFAIL,
			"%s geteuid() returns 0 unexpectedly, it gains root privileges",
			argv[1]);
	if (TST_RET == pw->pw_uid)
		tst_res(TPASS,
			"%s geteuid() returns nobody, it doesn't gain root privileges",
			argv[1]);

	return 0;
}
