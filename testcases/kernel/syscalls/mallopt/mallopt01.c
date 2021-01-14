// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) Linux Test Project, 2003-2021
 * Copyright (c) International Business Machines  Corp., 2002
 * 01/02/2003	Port to LTP	<avenkat@us.ibm.com>
 * 06/30/2001	Port to Linux <nsharoff@us.ibm.com>
 */

/*\
 * [DESCRIPTION]
 *
 * Basic mallinfo() and mallopt() testing.
\*/

#include <malloc.h>

#include "tst_test.h"
#include "tst_safe_macros.h"

#ifdef HAVE_MALLOPT

#define MAX_FAST_SIZE	(80 * sizeof(size_t) / 4)

struct mallinfo info;

void print_mallinfo(void)
{
	tst_res(TINFO, "mallinfo structure:");
	tst_res(TINFO, "mallinfo.arena = %d", info.arena);
	tst_res(TINFO, "mallinfo.ordblks = %d", info.ordblks);
	tst_res(TINFO, "mallinfo.smblks = %d", info.smblks);
	tst_res(TINFO, "mallinfo.hblkhd = %d", info.hblkhd);
	tst_res(TINFO, "mallinfo.hblks = %d", info.hblks);
	tst_res(TINFO, "mallinfo.usmblks = %d", info.usmblks);
	tst_res(TINFO, "mallinfo.fsmblks = %d", info.fsmblks);
	tst_res(TINFO, "mallinfo.uordblks = %d", info.uordblks);
	tst_res(TINFO, "mallinfo.fordblks = %d", info.fordblks);
	tst_res(TINFO, "mallinfo.keepcost = %d", info.keepcost);
}

void test_mallopt(void)
{
	char *buf;

	buf = SAFE_MALLOC(20480);

	info = mallinfo();
	if (info.uordblks < 20480) {
		print_mallinfo();
		tst_res(TFAIL, "mallinfo() failed: uordblks < 20K");
	}
	if (info.smblks != 0) {
		print_mallinfo();
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
