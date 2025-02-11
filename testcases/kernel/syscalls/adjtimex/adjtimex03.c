// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) Zilogic Systems Pvt. Ltd <code@zilogic.com>, 2020
 * Copyright (c) Linux Test Project, 2021-2023
 *
 * Based on testcases/kernel/syscalls/adjtimex/adjtimex01.c
 * Copyright (c) Wipro Technologies Ltd, 2002.
 */

/*\
 * CVE-2018-11508: Test 4-byte kernel data leak via adjtimex.
 *
 * On calling the adjtimex() function call with invalid mode (let's say
 * 0x8000), ideally all the parameters should return with null data. But,
 * when we read the last parameter we will receive 4 bytes of kernel data.
 * This proves that there are 4 bytes of info leaked. The bug was fixed in
 * Kernel Version 4.16.9. Therefore, the below test case will only be
 * applicable for the kernel version 4.16.9 and above.
 *
 * So basically, this test shall check whether there is any data leak.
 * To test that, Pass struct timex buffer filled with zero with
 * some INVALID mode to the system call adjtimex. Passing an invalid
 * parameters will not call do_adjtimex() and before that, it shall throw
 * an error (on error test shall not break). Therefore, none of the parameters
 * will get initialized.
 *
 * On reading the last attribute tai of the struct, if the attribute is non-
 * zero the test is considered to have failed, else the test is considered
 * to have passed.
 */

#include <errno.h>
#include <sys/timex.h>
#include "tst_test.h"

#define ADJ_ADJTIME 0x8000
#define LOOPS 10

static struct timex *buf;

void verify_adjtimex(void)
{
	int i;
	int data_leak = 0;

	for (i = 0; i < LOOPS; i++) {
		memset(buf, 0, sizeof(struct timex));
		buf->modes = ADJ_ADJTIME; /* Invalid mode */
		TEST(adjtimex(buf));
		if ((TST_RET == -1) && (TST_ERR == EINVAL)) {
			tst_res(TINFO,
				"expecting adjtimex() to fail with EINVAL with mode 0x%x",
				ADJ_ADJTIME);
		} else {
			tst_brk(TBROK | TERRNO,
				"adjtimex(): Unexpeceted error, expecting EINVAL with mode 0x%x",
				ADJ_ADJTIME);
		}

		tst_res(TINFO, "tai : 0x%08x", buf->tai);

		if (buf->tai != 0) {
			data_leak = 1;
			break;
		}
	}

	if (data_leak != 0)
		tst_res(TFAIL, "Data leak observed");
	else
		tst_res(TPASS, "Data leak not observed");
}

static struct tst_test test = {
	.test_all = verify_adjtimex,
	.bufs = (struct tst_buffers []) {
		{&buf, .size = sizeof(*buf)},
		{},
	},
	.tags = (const struct tst_tag[]) {
		{"CVE", "2018-11508"},
		{"linux-git", "0a0b98734479"},
		{},
	}
};
