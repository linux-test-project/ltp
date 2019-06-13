// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines Corp., 2001
 * Copyright (c) 2018 Petr Vorel <pvorel@suse.cz>
 * Author: David L Stevens
 *
 * IPv6 name to index and index to name function tests
 */


#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <net/if.h>

#include "tst_test.h"

#define I2N_RNDCOUNT	10	/* random ints */
#define I2N_LOWCOUNT	10	/* sequential from 0 */

static struct {
	char *name;
	int nonzero;
} test_case[] = {
	{ "lo", 1 },
	{ NULL, 1 },
	{ "hoser75", 0 },
	{ "6", 0 },
};

static void setup(void);
static void if_nametoindex_test(void);
static void if_indextoname_test(void);
static void if_nameindex_test(void);

static void (*testfunc[])(void) = { if_nametoindex_test, if_indextoname_test,
	if_nameindex_test };

static void if_nametoindex_test(void)
{
	unsigned int i;
	char ifname[IF_NAMESIZE], *pifn;

	tst_res(TINFO, "IPv6 if_nametoindex() test");

	for (i = 0; i < ARRAY_SIZE(test_case); ++i) {
		if (test_case[i].name == NULL) {
			tst_res(TCONF, "LHOST_IFACES not defined or invalid");
			continue;
		}

		TEST(if_nametoindex(test_case[i].name));
		if (!TST_RET != !test_case[i].nonzero) {
			tst_res(TFAIL, "if_nametoindex(%s) %ld [should be %szero]",
					test_case[i].name, TST_RET,
					test_case[i].nonzero ? "non" : "");
			return;
		}
		if (TST_RET) {
			pifn = if_indextoname(TST_RET, ifname);
			if (!pifn || strcmp(test_case[i].name, pifn)) {
				tst_res(TFAIL,
					"if_nametoindex(%s) %ld doesn't match if_indextoname(%ld) '%s'",
					test_case[i].name, TST_RET,
					TST_RET, pifn ? pifn : "");
				return;
			}
		}
		tst_res(TINFO, "if_nametoindex(%s) %ld",
			test_case[i].name, TST_RET);
	}

	tst_res(TPASS, "if_nametoindex() test succeeded");
}

static int sub_if_indextoname_test(unsigned int if_index)
{
	char ifname[IF_NAMESIZE];
	unsigned int idx;

	TEST((ifname == if_indextoname(if_index, ifname)));
	if (!TST_RET) {
		if (TST_ERR != ENXIO) {
			tst_res(TFAIL,
				"if_indextoname(%d) returns %ld but errno %d != ENXIO",
				if_index, TST_RET, TST_ERR);
			return 0;
		}
		tst_res(TINFO, "if_indextoname(%d) returns NULL", if_index);
		return 1;
	}
	/* else, a valid interface-- double check name */
	idx = if_nametoindex(ifname);
	if (idx != if_index) {
		tst_res(TFAIL,
			"if_indextoname(%u) returns '%s' but doesn't if_nametoindex(%s) returns %u",
			if_index, ifname, ifname, idx);
		return 0;
	}
	tst_res(TINFO, "if_indextoname(%d) returns '%s'", if_index, ifname);
	return 1;
}

static void if_indextoname_test(void)
{
	unsigned int i;

	tst_res(TINFO, "IPv6 if_indextoname() test");

	/* some low-numbered indexes-- likely to get valid interfaces here */
	for (i = 0; i < I2N_LOWCOUNT; ++i)
		if (!sub_if_indextoname_test(i))
			return;	/* skip the rest, if broken */
	/* some random ints; should mostly fail */
	for (i = 0; i < I2N_RNDCOUNT; ++i)
		if (!sub_if_indextoname_test(rand()))
			return;	/* skip the rest, if broken */

	tst_res(TPASS, "if_indextoname() test succeeded");
}

/*
 * This is an ugly, linux-only solution. getrusage() doesn't support the
 * current data segment size, so we get it out of /proc
 */
static int getdatasize(void)
{
	char line[128], *p;
	int dsize = -1;
	FILE *fp;

	fp = fopen("/proc/self/status", "r");
	if (fp == NULL)
		return -1;
	while (fgets(line, sizeof(line), fp)) {
		if (strncmp(line, "VmData:", 7) == 0) {
			dsize = strtol(line + 7, &p, 0);
			++p;	/* skip space */
			if (!strcmp(p, "kB"))
				return -1;	/* don't know units */
			dsize *= 1024;
			break;
		}
	}
	fclose(fp);
	return dsize;
}

static void if_nameindex_test(void)
{
	struct if_nameindex *pini;
	int i;
	char buf[IF_NAMESIZE], *p;
	unsigned int idx;
	int freenicount;
	int dsize_before, dsize_after;

	tst_res(TINFO, "IPv6 if_nameindex() test");

	pini = if_nameindex();
	if (pini == NULL) {
		tst_res(TFAIL, "if_nameindex() returns NULL, errno %d (%s)",
			TST_ERR, strerror(TST_ERR));
		return;
	}
	for (i = 0; pini[i].if_index; ++i) {
		p = if_indextoname(pini[i].if_index, buf);
		if (!p || strcmp(p, pini[i].if_name)) {
			tst_res(TFAIL,
				"if_nameindex() idx %d name '%s' but if_indextoname(%d) is '%s'",
				pini[i].if_index, pini[i].if_name,
				pini[i].if_index, p ? p : "");
			return;
		}
		idx = if_nametoindex(pini[i].if_name);
		if (idx != pini[i].if_index) {
			tst_res(TFAIL,
				"if_nameindex() idx %d name '%s' but if_indextoname(%s) is %d",
				pini[i].if_index, pini[i].if_name,
				pini[i].if_name, idx);
			return;
		}
		tst_res(TINFO, "if_nameindex() idx %d name '%s'",
				pini[i].if_index, pini[i].if_name);
	}
	if_freenameindex(pini);

	/*
	 * if_freenameindex() has no error conditions; see if we run
	 * out of memory if we do it a lot.
	 */
	dsize_before = getdatasize();
	if (dsize_before < 0) {
		tst_brk(TBROK, "getdatasize failed: errno %d (%s)",
			errno, strerror(errno));
	}

	/*
	 * we need to leak at least a page to detect a leak; 1 byte per call
	 * will be detected with getpagesize() calls.
	 */
	freenicount = getpagesize();
	for (i = 0; i < freenicount; ++i) {
		pini = if_nameindex();
		if (pini == NULL) {
			tst_res(TINFO,
				"if_freenameindex test failed if_nameindex() iteration %d", i);
			break;
		}
		if_freenameindex(pini);
	}
	dsize_after = getdatasize();
	if (dsize_after < 0) {
		tst_brk(TBROK, "getdatasize failed: errno %d (%s)",
			errno, strerror(errno));
	}
	if (dsize_after > dsize_before + getpagesize()) {
		tst_res(TFAIL,
			"if_freenameindex leaking memory (%d iterations) dsize before %d dsize after %d",
			i, dsize_before, dsize_after);
		return;
	}
	tst_res(TINFO, "if_freenameindex passed %d iterations", i);

	tst_res(TPASS, "if_nameindex() test succeeded");
}

static void setup(void)
{
	char *ifnames = getenv("LHOST_IFACES");

	if (!ifnames)
		return;

	static char name[256];
	int ret;

	ret = sscanf(ifnames, "%255s", name);
	if (ret == -1)
		return;

	tst_res(TINFO, "get interface name from LHOST_IFACES: '%s'", name);
	test_case[1].name = name;
}

static void do_test(unsigned int i)
{
	testfunc[i]();
}

static struct tst_test test = {
	.tcnt = ARRAY_SIZE(testfunc),
	.setup = setup,
	.test = do_test,
};
