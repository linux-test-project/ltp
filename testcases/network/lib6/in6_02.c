/*
 *
 *   Copyright (c) International Business Machines  Corp., 2001
 *   Author: David L Stevens
 *
 *   This program is free software;  you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY;  without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
 *   the GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program;  if not, write to the Free Software Foundation,
 *   Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */
/*
 *   Description:
 *     Tests for name to index and index to name functions in IPv6
 */

#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <net/if.h>

#include "test.h"

static struct {
	char *name;
	int nonzero;
} n2i[] = {
	{ "lo", 1 },
	{ "eth0", 1 },
	{ "hoser75", 0 },
	{ "6", 0 },
};

#define N2I_COUNT (sizeof(n2i)/sizeof(n2i[0]))
#define I2N_RNDCOUNT	10	/* random ints */
#define I2N_LOWCOUNT	10	/* sequential from 0 */

static void setup(void);
static void n2itest(void);
static void i2ntest(void);
static void initest(void);

static void (*testfunc[])(void) = { n2itest,
	i2ntest, initest };

char *TCID = "in6_02";
int TST_TOTAL = ARRAY_SIZE(testfunc);

int main(int argc, char *argv[])
{
	int lc;
	int i;

	tst_parse_opts(argc, argv, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); ++lc) {
		tst_count = 0;

		for (i = 0; i < TST_TOTAL; i++)
			(*testfunc[i])();
	}

	tst_exit();
}

/* if_nametoindex tests */
void n2itest(void)
{
	unsigned int i;
	char ifname[IF_NAMESIZE], *pifn;

	for (i = 0; i < N2I_COUNT; ++i) {
		TEST(if_nametoindex(n2i[i].name));
		if (!TEST_RETURN != !n2i[i].nonzero) {
			tst_resm(TFAIL, "if_nametoindex(\"%s\") %ld "
				"[should be %szero]", n2i[i].name, TEST_RETURN,
				n2i[i].nonzero ? "non" : "");
			return;
		}
		if (TEST_RETURN) {
			pifn = if_indextoname(TEST_RETURN, ifname);
			if (!pifn || strcmp(n2i[i].name, pifn)) {
				tst_resm(TFAIL, "if_nametoindex(\"%s\") %ld "
					"doesn't match if_indextoname(%ld) "
					"\"%s\"", n2i[i].name, TEST_RETURN,
					TEST_RETURN, pifn ? pifn : "");
				return;
			}
		}
		tst_resm(TINFO, "if_nametoindex(\"%s\") %ld",
			n2i[i].name, TEST_RETURN);
	}

	tst_resm(TPASS, "if_nametoindex() tests succeed");
}

int sub_i2ntest(unsigned int if_index)
{
	char ifname[IF_NAMESIZE];
	unsigned int idx;

	TEST((ifname == if_indextoname(if_index, ifname)));
	if (!TEST_RETURN) {
		if (TEST_ERRNO != ENXIO) {
			tst_resm(TFAIL, "if_indextoname(%d) returns %ld "
				 "but errno %d != ENXIO", if_index, TEST_RETURN,
				 TEST_ERRNO);
			return 0;
		}
		tst_resm(TINFO, "if_indextoname(%d) returns NULL", if_index);
		return 1;
	}
	/* else, a valid interface-- double check name */
	idx = if_nametoindex(ifname);
	if (idx != if_index) {
		tst_resm(TFAIL, "if_indextoname(%u) returns \"%s\" but "
			 "doesn't if_nametoindex(\"%s\") returns %u",
			 if_index, ifname, ifname, idx);
		return 0;
	}
	tst_resm(TINFO, "if_indextoname(%d) returns \"%s\"", if_index, ifname);
	return 1;
}

/* if_indextoname tests */
void i2ntest(void)
{
	unsigned int i;

	/* some low-numbered indexes-- likely to get valid interfaces here */
	for (i = 0; i < I2N_LOWCOUNT; ++i)
		if (!sub_i2ntest(i))
			return;	/* skip the rest, if broken */
	/* some random ints; should mostly fail */
	for (i = 0; i < I2N_RNDCOUNT; ++i)
		if (!sub_i2ntest(rand()))
			return;	/* skip the rest, if broken */

	tst_resm(TPASS, "if_indextoname() tests succeed");
}

/*
 * This is an ugly, linux-only solution. getrusage() doesn't support the
 * current data segment size, so we get it out of /proc
 */
int getdatasize(void)
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

/* if_nameindex tests */
void initest(void)
{
	struct if_nameindex *pini;
	int i;
	char buf[IF_NAMESIZE], *p;
	unsigned int idx;
	int freenicount;
	int dsize_before, dsize_after;

	pini = if_nameindex();
	if (pini == NULL) {
		tst_resm(TFAIL, "if_nameindex() returns NULL, errno %d (%s)",
			 TEST_ERRNO, strerror(TEST_ERRNO));
		return;
	}
	for (i = 0; pini[i].if_index; ++i) {
		p = if_indextoname(pini[i].if_index, buf);
		if (!p || strcmp(p, pini[i].if_name)) {
			tst_resm(TFAIL, "if_nameindex idx %d name \"%s\" but "
				 "if_indextoname(%d) is \"%s\"",
				 pini[i].if_index, pini[i].if_name,
				 pini[i].if_index, p ? p : "");
			return;
		}
		idx = if_nametoindex(pini[i].if_name);
		if (idx != pini[i].if_index) {
			tst_resm(TFAIL, "if_nameindex idx %d name \"%s\" but "
				 "if_indextoname(\"%s\") is %d",
				 pini[i].if_index, pini[i].if_name,
				 pini[i].if_name, idx);
			return;
		}
		tst_resm(TINFO, "if_nameindex idx %d name \"%s\"",
			 pini[i].if_index, pini[i].if_name);
	}
	if_freenameindex(pini);

	/* if_freenameindex() has no error conditions; see if we run
	 * out of memory if we do it a lot.
	 */
	dsize_before = getdatasize();
	if (dsize_before < 0) {
		tst_brkm(TBROK, NULL, "getdatasize failed: errno %d (%s)",
			errno, strerror(errno));
	}
	/* we need to leak at least a page to detect a leak; 1 byte per call
	 * will be detected with getpagesize() calls.
	 */
	freenicount = getpagesize();
	for (i = 0; i < freenicount; ++i) {
		pini = if_nameindex();
		if (pini == NULL) {
			tst_resm(TINFO, "if_freenameindex test failed "
				 "if_nameindex() iteration %d", i);
			break;
		}
		if_freenameindex(pini);
	}
	dsize_after = getdatasize();
	if (dsize_after < 0) {
		tst_brkm(TBROK, NULL, "getdatasize failed: errno %d (%s)",
			errno, strerror(errno));
	}
	if (dsize_after > dsize_before + getpagesize()) {
		tst_resm(TFAIL, "if_freenameindex leaking memory "
			 "(%d iterations) dsize before %d dsize after %d", i,
			 dsize_before, dsize_after);
		return;
	} else {
		tst_resm(TINFO, "if_freenameindex passed %d iterations", i);
	}

	tst_resm(TPASS, "if_nameindex() tests succeed");
}

void setup(void)
{
	TEST_PAUSE;

	tst_resm(TINFO, "get interface name from LHOST_IFACES var");

	char *ifnames = getenv("LHOST_IFACES");

	if (!ifnames) {
		tst_resm(TWARN, "LHOST_IFACES not defined, default to eth0");
		return;
	}

	static char name[256];

	sscanf(ifnames, "%255s", name);

	if (!strcmp(name, n2i[1].name))
		return;

	tst_resm(TINFO, "change default 'eth0' name to '%s'", name);
	n2i[1].name = name;
}
