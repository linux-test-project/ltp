// SPDX-License-Identifier: GPL-2.0-or-later
/*
 *   Copyright (c) 2015 Fujitsu Ltd.
 *   Author: Zeng Linggang <zenglg.jy@cn.fujitsu.com>
 *   Copyright (C) 2024 SUSE LLC Andrea Manzini <andrea.manzini@suse.com>
 */

/*\
 * Test for GHOST: glibc vulnerability (CVE-2015-0235).
 *
 * https://www.qualys.com/research/security-advisories/GHOST-CVE-2015-0235.txt
 */

#include "tst_test.h"

#define CANARY "in_the_coal_mine"

static struct
{
	char buffer[1024];
	char canary[sizeof(CANARY)];
} temp = {
	"buffer",
	CANARY,
};

static void check_vulnerable(void)
{
	struct hostent resbuf;
	struct hostent *result;
	int herrno;
	int retval;
	char name[sizeof(temp.buffer)];
	size_t len;

	/*
	 * <glibc>/nss/digits_dots.c:
	 * strlen(name) = size_needed - sizeof(*host_addr) -
	 *                sizeof(*h_addr_ptrs) - 1;
	 */
	len = sizeof(temp.buffer) - 16 - 2 * sizeof(char *) - 1;

	memset(name, '0', len);
	name[len] = '\0';
	retval = gethostbyname_r(name, &resbuf, temp.buffer,
				 sizeof(temp.buffer), &result, &herrno);

	/* has canary been overwritten? */
	if (strcmp(temp.canary, CANARY) != 0)
		tst_res(TFAIL, "GHOST CVE-2015-0235 vulnerable");
	else
		TST_EXP_EQ_LI(retval, ERANGE);
}

static struct tst_test test = {
	.test_all = check_vulnerable,
	.tags = (const struct tst_tag[]) {
		{"glibc-git", "d5dd6189d506"},
		{"CVE", "CVE-2015-0235"},
		{}
	}
};
