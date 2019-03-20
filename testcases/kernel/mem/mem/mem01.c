/*
 * mem01.c - Basic memory and swapper stress test
 *
 * Copyright (C) 2001 Stephane Fillod <f4cfe@free.fr>
 * Original idea from Rene Cougnenc
 *
 * Copyright (C) 2012 Cyril Hrubis <chrubis@suse.cz>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it would be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * Further, this software is distributed without any warranty that it is
 * free of the rightful claim of any third person regarding infringement
 * or the like.  Any license provided herein, whether implied or
 * otherwise, applies only to this software file.  Patent licenses, if
 * any, provided herein do not apply to combinations of this program with
 * other software, or any other product whatsoever.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 */

#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/sysinfo.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/user.h>
#include <time.h>
#include <limits.h>

#include "lapi/abisize.h"
#include "test.h"

/* in KB */
#define PROGRESS_LEAP 100

/*
 * TODO:
 *  - add option for growing direction, when doing linear touching
 *  - add option for touch running time (or infinite loop?)
 *  - make it multithreaded with random access to test r/w mm_sem
 */

char *TCID = "mem01";
int TST_TOTAL = 1;

static int m_opt = 0;		/* memsize */
static char *m_copt;

static int r_opt = 0;		/* random access versus linear */
static int v_opt = 0;		/* verbose progress indication */

static void cleanup(void)
{
	tst_rmdir();
	tst_exit();
}

static void setup(void)
{
	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;

	tst_tmpdir();
}

static void help(void)
{
	printf("  -m x    size of malloc in MB (default from /proc/meminfo)\n");
	printf("  -r      random touching versus linear\n");
	printf("  -v      verbose progress indication\n");
}

/*
 * return MemFree+SwapFree, from /proc/meminfo
 * returned value is in bytes.
 */
size_t get_memsize(void)
{
	struct sysinfo info;
	unsigned long long res;
	unsigned long long freeswap;
	unsigned long long freeram;
	int ret;

	ret = sysinfo(&info);
	if (ret != 0) {
		tst_resm(TFAIL,
			 "Could not retrieve memory information using sysinfo()");
		cleanup();
	}

	freeram =
	    (unsigned long long)info.freeram *
	    (unsigned long long)info.mem_unit;
	tst_resm(TINFO, "Free Mem:\t%llu Mb", freeram / 1024 / 1024);
	res = freeram;

	freeswap =
	    (unsigned long long)info.freeswap *
	    (unsigned long long)info.mem_unit;
	tst_resm(TINFO, "Free Swap:\t%llu Mb", freeswap / 1024 / 1024);
	res = res + freeswap;

	tst_resm(TINFO, "Total Free:\t%llu Mb", res / 1024 / 1024);
#if defined(__s390__)
	if (res > 1 * 1024 * 1024 * 1024)
		res = 500 * 1024 * 1024;	/* s390's unique 31bit architecture needs smaller default */
#elif defined(TST_ABI32)
	if (res > 1 * 1024 * 1024 * 1024)
		res = 1 * 1024 * 1024 * 1024;
#elif defined(TST_ABI64)
	if (res > (unsigned long long)3 * 1024 * 1024 * 1024)
		res = (unsigned long long)3 *1024 * 1024 * 1024;
#endif

	/* Always reserve 16MB memory to avoid OOM Killer. */
	res -= 16 * 1024 * 1024;
	tst_resm(TINFO, "Total Tested:\t%llu Mb", res / 1024 / 1024);
	return (size_t) res;
}

/*
 * add the -m option whose parameter is the
 * memory size (MB) to allocate.
 */
option_t options[] = {
	{"m:", &m_opt, &m_copt}
	,
	{"r", &r_opt, NULL}
	,
	{"v", &v_opt, NULL}
	,
	{NULL, NULL, NULL}
};

int main(int argc, char *argv[])
{
	size_t memsize = 0;	/* at first in MB, limited to 4Gb on 32 bits */
	int pagesize;

	int i;
	int lc;
	char *p, *bigmalloc;
	int loop_count;		/* limited to 16Go on 32 bits systems */

	pagesize = sysconf(_SC_PAGESIZE);

	tst_parse_opts(argc, argv, options, help);

	if (m_opt) {
		memsize = (size_t) atoi(m_copt) * 1024 * 1024;

		if (memsize < 1) {
			tst_brkm(TBROK, cleanup, "Invalid arg for -m: %s",
				 m_copt);
		}
	}

	if (r_opt)
		srand(time(NULL));

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {

		tst_count = 0;

		if (!m_opt) {
			/* find out by ourselves! */
			memsize = get_memsize();
			if (memsize < 1) {
				tst_brkm(TBROK, cleanup,
					 "Unable to guess maxmemsize from /proc/meminfo");
			}
		}

		/* Allocate (virtual) memory */
		bigmalloc = p = malloc(memsize);

		if (!p) {
			tst_resm(TFAIL, "malloc - alloc of %zuMB failed",
				 memsize / 1024 / 1024);
			cleanup();
		}

		/*
		 * Dirty all the pages, to force physical RAM allocation
		 * and exercise eventually the swapper
		 */
		tst_resm(TINFO, "touching %zuMB of malloc'ed memory (%s)",
			 memsize / 1024 / 1024, r_opt ? "random" : "linear");

		loop_count = memsize / pagesize;

		for (i = 0; i < loop_count; i++) {
			if (v_opt
			    && (i % (PROGRESS_LEAP * 1024 / pagesize) == 0)) {
				printf(".");
				fflush(stdout);
			}
			/*
			 * Make the page dirty,
			 * and make sure compiler won't optimize it away
			 * Touching more than one word per page is useless
			 * because of cache.
			 */
			*(int *)p = 0xdeadbeef ^ i;

			if (r_opt) {
				p = bigmalloc +
				    (size_t) ((double)(memsize - sizeof(int)) *
					      rand() / (RAND_MAX + 1.0));
			} else {
				p += pagesize;
			}
		}

		if (v_opt)
			printf("\n");

		/* This is not mandatory (except in a loop), but it exercise mm again */
		free(bigmalloc);

		/*
		 * seems that if the malloc'ed area was bad, we'd get SEGV (or kicked
		 * somehow by the OOM killer?), hence we can indicate a PASS.
		 */
		tst_resm(TPASS, "malloc - alloc of %zuMB succeeded",
			 memsize / 1024 / 1024);

	}

	cleanup();

	return 0;
}
