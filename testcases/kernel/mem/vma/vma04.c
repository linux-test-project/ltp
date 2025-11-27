/*
 * Copyright (C) 2012 Linux Test Project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of version 2 of the GNU General Public
 * License as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it would be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * Further, this software is distributed without any warranty that it
 * is free of the rightful claim of any third person regarding
 * infringement or the like.  Any license provided herein, whether
 * implied or otherwise, applies only to this software file.  Patent
 * licenses, if any, provided herein do not apply to combinations of
 * this program with other software, or any other product whatsoever.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */
/*
 * There are several corner cases (documented in mm/mmap.c) for mbind
 * vma merge issue, which makes commit 8aacc9f550 slightly incorrect.
 * KOSAKI Motohiro made a patch for it (commit e26a511) and composed
 * a reproducer containing these corner cases. Now I port it to LTP.
 *
 * Author: KOSAKI Motohiro <kosaki.motohiro@jp.fujitsu.com>
 * Ported-to-LTP-by: Caspar Zhang <caspar@casparzhang.com>
 */

#include "config.h"
#include <sys/types.h>
#include <sys/mman.h>
#include <errno.h>
#if HAVE_NUMA_H
#include <numa.h>
#endif
#if HAVE_NUMAIF_H
#include <numaif.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include "test.h"
#include "safe_macros.h"
#include "numa_helper.h"

char *TCID = "vma04";
int TST_TOTAL = 5;

#ifdef HAVE_NUMA_V2

static unsigned long pagesize;
static int opt_node;
static char *opt_nodestr;
static char retbuf[BUFSIZ];
static void *mmap_addr;
static struct bitmask *nmask;

static option_t options[] = {
	{"n:", &opt_node, &opt_nodestr},
	{NULL, NULL, NULL}
};

static void init(void);
static void fin(void);
static void mem_bind(int index, int len);
static void mem_interleave(int index, int len);
static void mem_unbind(int index, int len);
static void assertion(char *expected, char *value, char *name);
static void get_vmas(char *retbuf, void *addr_s, void *addr_e);
static void case4(void);
static void case5(void);
static void case6(void);
static void case7(void);
static void case8(void);
static void setup(void);
static void cleanup(void);
static void usage(void);

int main(int argc, char **argv)
{
	int lc, node, err;

	tst_parse_opts(argc, argv, options, usage);

	nmask = numa_allocate_nodemask();
	if (opt_node) {
		node = SAFE_STRTOL(NULL, opt_nodestr, 1, LONG_MAX);
	} else {
		err = get_allowed_nodes(NH_MEMS | NH_MEMS, 1, &node);
		if (err == -3)
			tst_brkm(TCONF, NULL, "requires at least one node.");
		else if (err < 0)
			tst_brkm(TBROK | TERRNO, NULL, "get_allowed_nodes");
	}
	numa_bitmask_setbit(nmask, node);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		tst_count = 0;

		case4();
		case5();
		case6();
		case7();
		case8();
	}

	cleanup();
	tst_exit();
}

/*
 *  BBBBBB
 * AAAAAAAA
 */
static void init(void)
{
	void *addr;

	addr = SAFE_MMAP(cleanup, NULL, pagesize * 8, PROT_NONE,
			 MAP_ANON | MAP_PRIVATE, 0, 0);
	SAFE_MMAP(cleanup, addr + pagesize, pagesize * 6,
		  PROT_READ | PROT_WRITE, MAP_ANON | MAP_PRIVATE | MAP_FIXED, 0,
		  0);

	mmap_addr = addr + pagesize;
	memset(mmap_addr, 0, pagesize * 6);
}

static void fin(void)
{
	void *addr;

	addr = mmap_addr - pagesize;
	SAFE_MUNMAP(cleanup, addr, pagesize * 8);

	memset(retbuf, 0, sizeof(retbuf));
}

static void mem_bind(int index, int len)
{
	if (mbind(mmap_addr + pagesize * index, pagesize * len,
		  MPOL_BIND, nmask->maskp, nmask->size, 0) != 0) {
		if (errno != ENOSYS)
			tst_brkm(TBROK | TERRNO, cleanup, "mbind: bind");
		else
			tst_brkm(TCONF, cleanup,
				 "mbind syscall not implemented "
				 "on this system.");
	}
}

static void mem_interleave(int index, int len)
{
	if (mbind(mmap_addr + pagesize * index, pagesize * len,
		  MPOL_INTERLEAVE, nmask->maskp, nmask->size, 0) != 0) {
		if (errno != ENOSYS)
			tst_brkm(TBROK | TERRNO, cleanup, "mbind: interleave");
		else
			tst_brkm(TCONF, cleanup,
				 "mbind syscall not implemented "
				 "on this system.");
	}
}

static void mem_unbind(int index, int len)
{
	if (mbind(mmap_addr + pagesize * index, pagesize * len,
		  MPOL_DEFAULT, NULL, 0, 0) != 0) {
		if (errno != ENOSYS)
			tst_brkm(TBROK | TERRNO, cleanup, "mbind: unbind");
		else
			tst_brkm(TCONF, cleanup,
				 "mbind syscall not implemented "
				 "on this system.");
	}
}

static void assertion(char *expected, char *value, char *name)
{
	if (strcmp(expected, value) == 0)
		tst_resm(TPASS, "%s: passed.", name);
	else
		tst_resm(TFAIL, "%s: failed. expect '%s', actual '%s'",
			 name, expected, value);
}

static void get_vmas(char *retbuf, void *addr_s, void *addr_e)
{
	FILE *fp;
	void *s, *t;
	char buf[BUFSIZ], tmpstr[BUFSIZ];
	int flag;

	retbuf[0] = '\0';
	flag = 0;
	fp = SAFE_FOPEN(NULL, "/proc/self/maps", "r");

	while (fgets(buf, BUFSIZ, fp) != NULL) {
		if (sscanf(buf, "%p-%p ", &s, &t) != 2)
			continue;
		if (addr_s <= s && s < addr_e) {
			if (!flag) {
				sprintf(tmpstr, "%ld", (t - s) / pagesize);
				flag = 1;
			} else {
				sprintf(tmpstr, ",%ld", (t - s) / pagesize);
			}
			strncat(retbuf, tmpstr, 32);
		}
	}
	fclose(fp);
}

/*
 *   AAAA
 * PPPPPPNNNNNN
 * might become
 * PPNNNNNNNNNN
 * case 4 below
 */
static void case4(void)
{
	init();
	mem_bind(0, 4);
	mem_unbind(2, 2);
	get_vmas(retbuf, mmap_addr, mmap_addr + pagesize * 6);
	assertion("2,4", retbuf, "case4");
	fin();
}

/*
 *       AAAA
 * PPPPPPNNNNNN
 * might become
 * PPPPPPPPPPNN
 * case 5 below
 */
static void case5(void)
{
	init();
	mem_bind(0, 2);
	mem_bind(2, 2);
	get_vmas(retbuf, mmap_addr, mmap_addr + pagesize * 6);
	assertion("4,2", retbuf, "case5");
	fin();
}

/*
 *     AAAA
 * PPPPNNNNXXXX
 * might become
 * PPPPPPPPPPPP 6
 */
static void case6(void)
{
	init();
	mem_bind(0, 2);
	mem_bind(4, 2);
	mem_bind(2, 2);
	get_vmas(retbuf, mmap_addr, mmap_addr + pagesize * 6);
	assertion("6", retbuf, "case6");
	fin();
}

/*
 *     AAAA
 * PPPPNNNNXXXX
 * might become
 * PPPPPPPPXXXX 7
 */
static void case7(void)
{
	init();
	mem_bind(0, 2);
	mem_interleave(4, 2);
	mem_bind(2, 2);
	get_vmas(retbuf, mmap_addr, mmap_addr + pagesize * 6);
	assertion("4,2", retbuf, "case7");
	fin();
}

/*
 *     AAAA
 * PPPPNNNNXXXX
 * might become
 * PPPPNNNNNNNN 8
 */
static void case8(void)
{
	init();
	mem_bind(0, 2);
	mem_interleave(4, 2);
	mem_interleave(2, 2);
	get_vmas(retbuf, mmap_addr, mmap_addr + pagesize * 6);
	assertion("2,4", retbuf, "case8");
	fin();
}

static void setup(void)
{
	tst_sig(FORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;

	pagesize = getpagesize();
}

static void cleanup(void)
{
}

static void usage(void)
{
	printf("  -n      Number of NUMA nodes\n");
}

#else
int main(void)
{
	tst_brkm(TCONF, NULL, NUMA_ERROR_MSG);
}
#endif
