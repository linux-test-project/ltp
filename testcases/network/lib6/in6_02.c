/*
 *
 *   Copyright (c) International Business Machines  Corp., 2001
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
 *   along with this program;  if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

/*
 * Test Name: in6_02
 *
 * Test Description:
 *  Tests for name to index and index to name functions in IPv6
 *
 * Usage:  <for command-line>
 *  in6_02 [-c n] [-e] [-i n] [-I x] [-P x] [-t]
 *     where,  -c n : Run n copies concurrently.
 *             -e   : Turn on errno logging.
 *	       -i n : Execute test n times.
 *	       -I x : Execute test for x seconds.
 *	       -P x : Pause for x seconds between iterations.
 *	       -t   : Turn on syscall timing.
 *
 * HISTORY
 *	05/2004 written by David L Stevens
 *
 * RESTRICTIONS:
 *  None.
 *
 */

#include <unistd.h>
#include <errno.h>

#include <sys/socket.h>

#include <net/if.h>

#include "test.h"
#include "usctest.h"

char *TCID="in6_02";		/* Test program identifier.    */
int testno;

void setup(void), cleanup(void);

struct {
	char	*name;
	int	nonzero;
} n2i[] = {
	{ "lo", 1 },
	{ "eth0", 1 },
	{ "hoser75", 0 },
	{ "6", 0 },
};

#define N2I_COUNT (sizeof(n2i)/sizeof(n2i[0]))

#define I2N_RNDCOUNT	10	/* random ints */
#define I2N_LOWCOUNT	10	/* sequential from 0 */

extern int Tst_count;

int TST_TOTAL = N2I_COUNT;

void n2itest(void), i2ntest(void), initest(void);

int
main(int argc, char *argv[])
{
	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */

	/* Parse standard options given to run the test. */
	msg = parse_opts(argc, argv, (option_t *)NULL, NULL);
	if (msg != (char *)NULL) {
		tst_brkm(TBROK, tst_exit, "OPTION PARSING ERROR - %s", msg);
	}

	setup();

	/* Check looping state if -i option given */
	for (lc = 0; TEST_LOOPING(lc); ++lc) {
		n2itest();
		i2ntest();
		initest();
	}
	cleanup();

	/* NOTREACHED */
	return(0);
}	/* End main */


/* if_nametoindex tests */

void
n2itest()
{
	int i;

	for (i=0; i < N2I_COUNT; ++i) {
		char ifname[IF_NAMESIZE], *pifn;
		int fail;

		TEST(if_nametoindex(n2i[i].name));
		fail = !TEST_RETURN != !n2i[i].nonzero;
		if (fail) {
			tst_resm(TFAIL, "if_nametoindex(\"%s\") %ld "
				"[should be %szero]", n2i[i].name,
				TEST_RETURN, n2i[i].nonzero ? "non" : "");
			continue;
		}
		if (!TEST_RETURN) {
			tst_resm(TPASS, "if_nametoindex(\"%s\") %ld",
				n2i[i].name, TEST_RETURN);
			continue;
		}

		pifn = if_indextoname(TEST_RETURN, ifname);
		if (!pifn || strcmp(n2i[i].name, pifn)) {
			tst_resm(TFAIL, "if_nametoindex(\"%s\") %ld doesn't "
				"match if_indextoname(%ld) \"%s\"", n2i[i].name,
				TEST_RETURN, TEST_RETURN, pifn ? pifn : "");
			continue;
		}
		tst_resm(TPASS, "if_nametoindex(\"%s\") %ld", n2i[i].name,
			TEST_RETURN);
	}
}

int
i2ntest1(unsigned int if_index)
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
		tst_resm(TPASS, "if_indextoname(%d) returns NULL", if_index);
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
	tst_resm(TPASS, "if_indextoname(%d) returns \"%s\"", if_index,
		ifname);
	return 1;
}

void
i2ntest(void)
{
	unsigned int i;

	/* some low-numbered indexes-- likely to get valid interfaces here */
	for (i=0; i<I2N_LOWCOUNT; ++i)
		if (!i2ntest1(i))
			return;		/* skip the rest, if broken */
	/* some random ints; should mostly fail */
	for (i=0; i<I2N_RNDCOUNT; ++i)
		if (!i2ntest1(rand()))
			return;		/* skip the rest, if broken */
}

/*
 * This is an ugly, linux-only solution. getrusage() doesn't support the
 * current data segment size, so we get it out of /proc
 */
static int
getdatasize(void)
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
			++p; /* skip space */
			if (!strcmp(p, "kB"))
				return -1;	/* don't know units */
			dsize *= 1024;
			break;
		}
	}
	fclose(fp);
	return dsize;
}

void
initest(void)
{
	int freenicount;
	struct if_nameindex *pini;
	int i, dsize_before, dsize_after;

	pini = if_nameindex();
	if (pini == 0) {
		tst_resm(TFAIL, "if_nameindex() returns NULL, errno %d (%s)",
			TEST_ERRNO, strerror(TEST_ERRNO));
		return;
	}
	for (i=0; pini[i].if_index; ++i) {
		char buf[IF_NAMESIZE], *p;
		int idx;

		p = if_indextoname(pini[i].if_index, buf);
		if (!p || strcmp(p, pini[i].if_name)) {
			tst_resm(TFAIL, "if_nameindex idx %d name \"%s\" but "
				"if_indextoname(%d) is \"%s\"",
				pini[i].if_index, pini[i].if_name,
				pini[i].if_index, p ? p : "");
			continue;
		}
		idx = if_nametoindex(pini[i].if_name);
		if (idx != pini[i].if_index) {
			tst_resm(TFAIL, "if_nameindex idx %d name \"%s\" but "
				"if_indextoname(\"%s\") is %d",
				pini[i].if_index, pini[i].if_name,
				pini[i].if_name, idx);
			continue;
		}
		tst_resm(TPASS, "if_nameindex idx %d name \"%s\"",
			pini[i].if_index, pini[i].if_name);
	}
	if_freenameindex(pini);

	/* if_freenameindex() has no error conditions; see if we run
	 * out of memory if we do it a lot.
	 */
	dsize_before = getdatasize();
	if (dsize_before < 0) {
		tst_resm(TBROK, "getdatasize failed: errno %d (%s)", errno,
			strerror(errno));
		return;
	}
	/* we need to leak at least a page to detect a leak; 1 byte per call
	 * will be detected with getpagesize() calls.
	 */
	freenicount = getpagesize();
	for (i=0; i<freenicount; ++i) {
		pini = if_nameindex();
		if (!pini) {
			tst_resm(TFAIL, "if_freenameindex test failed "
				"if_nameindex() iteration %d", i);
			break;
		}
		if_freenameindex(pini);
	}
	dsize_after = getdatasize();
	if (dsize_after < 0) {
		tst_resm(TBROK, "getdatasize failed: errno %d (%s)", errno,
			strerror(errno));
		return;
	}
	if (dsize_after > dsize_before + getpagesize())
		tst_resm(TFAIL, "if_freenameindex leaking memory "
			"(%d iterations) dsize before %d dsize after %d", i,
			dsize_before, dsize_after);
	else
		tst_resm(TPASS, "if_freenameindex passed %d iterations", i);
}

void
setup(void)
{
	TEST_PAUSE;	/* if -P option specified */
}

void
cleanup(void)
{
	TEST_CLEANUP;
	tst_exit();
}

