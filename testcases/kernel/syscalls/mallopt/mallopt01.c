// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) Linux Test Project, 2003-2021
 * Copyright (c) International Business Machines  Corp., 2002
 * 01/02/2003	Port to LTP	<avenkat@us.ibm.com>
 * 06/30/2001	Port to Linux <nsharoff@us.ibm.com>
 */

/*\
 * Basic mallinfo() and mallopt() testing.
 */


#include "../mallinfo/mallinfo_common.h"
#include "tst_safe_macros.h"

#ifdef HAVE_MALLOPT

#define MAX_FAST_SIZE	(80 * sizeof(size_t) / 4)

struct mallinfo info;

void test_mallopt(void)
{
	char *buf;

	buf = SAFE_MALLOC(20480);

	info = mallinfo();
	if (info.uordblks < 20480) {
		print_mallinfo("Test uordblks", &info);
		tst_res(TFAIL, "mallinfo() failed: uordblks < 20K");
	}
	if (info.smblks != 0) {
		print_mallinfo("Test smblks", &info);
		tst_res(TFAIL, "mallinfo() failed: smblks != 0");
	}
	if (info.uordblks >= 20480 && info.smblks == 0)
		tst_res(TPASS, "mallinfo() succeeded");

	free(buf);

	if (mallopt(M_MXFAST, MAX_FAST_SIZE) == 0)
		tst_res(TFAIL, "mallopt(M_MXFAST, %d) failed", (int)MAX_FAST_SIZE);
	else
		tst_res(TPASS, "mallopt(M_MXFAST, %d) succeeded", (int)MAX_FAST_SIZE);

	if ((buf = malloc(1024)) == NULL) {
		tst_res(TFAIL, "malloc(1024) failed");
	} else {
		tst_res(TPASS, "malloc(1024) succeeded");
		free(buf);
	}

	if (mallopt(M_MXFAST, 0) == 0)
		tst_res(TFAIL, "mallopt(M_MXFAST, 0) failed");
	else
		tst_res(TPASS, "mallopt(M_MXFAST, 0) succeeded");

	if ((buf = malloc(1024)) == NULL) {
		tst_res(TFAIL, "malloc(1024) failed");
	} else {
		tst_res(TPASS, "malloc(1024) succeeded");
		free(buf);
	}
}

static struct tst_test test = {
	.test_all = test_mallopt,
};

#else
TST_TEST_TCONF("system doesn't implement non-POSIX mallopt()");
#endif
